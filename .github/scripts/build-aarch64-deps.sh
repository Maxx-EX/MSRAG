#!/usr/bin/env bash
set -euo pipefail
set -x

PROJECT_ROOT="${1:?请传入项目根目录}"
PREFIX="${DEPS_PREFIX:?请设置 DEPS_PREFIX 环境变量}"
TOOLCHAIN="$PROJECT_ROOT/cmake/aarch64-linux-gnu.cmake"

export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
export CFLAGS="-I$PREFIX/include"
export LDFLAGS="-L$PREFIX/lib"
export MAKEFLAGS="-j$(nproc)"

mkdir -p "$PREFIX"
BUILD_DIR="$(mktemp -d)"
cd "$BUILD_DIR"

# 增强版下载函数：自动尝试官方源 -> 国内 GitHub 加速镜像 -> curl 官方源
fetch() {
    local url="$1" out="$2"
    echo ">>> 正在下载 $out"
    
    # 1. 尝试标准 curl (限时 20 秒)
    if curl -fL --retry 2 --retry-delay 2 --connect-timeout 20 -o "$out" "$url"; then
        return 0
    fi
    
    echo ">>> 标准源下载失败，尝试国内加速镜像..."
    # 2. 如果是 GitHub 链接，尝试 ghproxy 镜像
    if [[ "$url" == *"github.com"* ]]; then
        local proxy_url="https://ghproxy.net/$url"
        if curl -fL --retry 2 --connect-timeout 20 -o "$out" "$proxy_url"; then
            echo ">>> 镜像下载成功！"
            return 0
        fi
    fi
    
    # 3. 再次尝试直接用 wget (应对某些 curl 不兼容的服务器)
    echo ">>> 尝试 wget 备用源..."
    if wget -q --timeout=20 --tries=2 -O "$out" "$url"; then
        return 0
    fi
    
    echo "!!! 彻底下载失败: $url"
    exit 1
}

# 智能解压并进入目录函数：解决解压出的目录名带版本号/前缀不同的痛点
extract_and_cd() {
    local file="$1"
    tar xf "$file"
    # 获取压缩包内第一层目录名，并进入
    local dir_name=$(tar tf "$file" | head -n 1 | cut -d/ -f1)
    cd "$dir_name"
}

# ---------- zlib ----------
fetch https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz zlib.tar.gz
extract_and_cd zlib.tar.gz
CHOST=aarch64-linux-gnu ./configure --prefix="$PREFIX" --static
make
make install
cd "$BUILD_DIR"

# ---------- libpng ----------
fetch https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.43.tar.gz libpng.tar.gz
extract_and_cd libpng.tar.gz
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static \
  CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make
make install
cd "$BUILD_DIR"

# ---------- freetype ----------
fetch https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-2.tar.gz freetype.tar.gz
extract_and_cd freetype.tar.gz
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static \
  --with-zlib=yes --with-png=yes --with-harfbuzz=no \
  CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make
make install
cd "$BUILD_DIR"

# ---------- SDL2 ----------
fetch https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-2.30.8.tar.gz SDL2.tar.gz
extract_and_cd SDL2.tar.gz
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_PREFIX_PATH="$PREFIX" \
  -DCMAKE_BUILD_TYPE=Release \
  -DSDL_STATIC=ON -DSDL_SHARED=ON \
  -DSDL_TEST=OFF -DSDL_TESTS=OFF \
  -DSDL_X11=OFF -DSDL_WAYLAND=OFF -DSDL_KMSDRM=OFF \
  -DSDL_DUMMYVIDEO=ON -DSDL_OFFSCREEN=ON
cmake --build build
cmake --install build
cd "$BUILD_DIR"

# ---------- SDL2_ttf ----------
fetch https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz SDL2_ttf.tar.gz
extract_and_cd SDL2_ttf.tar.gz
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_PREFIX_PATH="$PREFIX" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
cmake --build build
cmake --install build

set +x
echo "========== 所有 aarch64 依赖构建完成 =========="
ls -lh "$PREFIX/lib"
