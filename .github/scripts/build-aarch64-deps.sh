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

# 增强版下载：支持多源回退
fetch() {
    local out="$1"
    shift
    local urls=("$@")
    for url in "${urls[@]}"; do
        echo ">>> 尝试下载: $url"
        if curl -fL --retry 3 --retry-delay 2 --connect-timeout 15 -o "$out" "$url"; then
            return 0
        fi
        if [[ "$url" == *"github.com"* ]]; then
            local proxy_url="https://ghproxy.net/$url"
            echo ">>> 尝试镜像: $proxy_url"
            if curl -fL --retry 3 --connect-timeout 15 -o "$out" "$proxy_url"; then
                return 0
            fi
        fi
    done
    echo "!!! 彻底下载失败: $out"
    exit 1
}

extract_and_cd() {
    tar xf "$1"
    local dir_name=$(tar tf "$1" | head -n 1 | cut -d/ -f1)
    cd "$dir_name"
}

# ---------- zlib (SDL2 可能用到) ----------
fetch zlib.tar.gz \
    https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
extract_and_cd zlib.tar.gz
CHOST=aarch64-linux-gnu ./configure --prefix="$PREFIX" --static
make
make install
cd "$BUILD_DIR"

# ---------- freetype (彻底禁用 zlib 和 png 防止链接错误) ----------
fetch freetype.tar.xz \
    https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.xz \
    https://mirrors.tuna.tsinghua.edu.cn/gnu/freetype/freetype-2.13.2.tar.xz \
    https://mirrors.ustc.edu.cn/gnu/freetype/freetype-2.13.2.tar.xz
extract_and_cd freetype.tar.xz
export FREETYPE_VERSION=2.13.2
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static \
  --with-zlib=no --with-png=no --with-harfbuzz=no \
  CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make
make install
cd "$BUILD_DIR"

# ---------- SDL2 ----------
fetch SDL2.tar.gz \
    https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-2.30.8.tar.gz
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
fetch SDL2_ttf.tar.gz \
    https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz
extract_and_cd SDL2_ttf.tar.gz
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_PREFIX_PATH="$PREFIX" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE \
  -DCMAKE_SHARED_LINKER_FLAGS="-L$PREFIX/lib -lz"
cmake --build build
cmake --install build

set +x
echo "========== 所有 aarch64 依赖构建完成 =========="
ls -lh "$PREFIX/lib"
