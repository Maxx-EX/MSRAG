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

fetch() {
    local url="$1" out="$2"
    echo "下载 $url"
    curl -fL --retry 5 --retry-delay 3 --connect-timeout 30 -o "$out" "$url"
}

# ---------- zlib ----------
fetch https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz zlib-1.3.1.tar.gz
tar xf zlib-1.3.1.tar.gz
cd zlib-1.3.1
CHOST=aarch64-linux-gnu ./configure --prefix="$PREFIX" --static
make
make install
cd "$BUILD_DIR"

# ---------- libpng ----------
fetch https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.43.tar.gz libpng-1.6.43.tar.gz
tar xf libpng-1.6.43.tar.gz
cd libpng-1.6.43
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static \
  CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make
make install
cd "$BUILD_DIR"

# ---------- freetype ----------
fetch https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-2.tar.gz freetype-2.13.2.tar.gz
tar xf freetype-2.13.2.tar.gz
cd freetype-2.13.2
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static \
  --with-zlib=yes --with-png=yes --with-harfbuzz=no \
  CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make
make install
cd "$BUILD_DIR"

# ---------- SDL2 ----------
fetch https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-2.30.8.tar.gz SDL2-2.30.8.tar.gz
tar xf SDL2-2.30.8.tar.gz
cd SDL2-2.30.8
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
fetch https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz SDL2_ttf-2.22.0.tar.gz
tar xf SDL2_ttf-2.22.0.tar.gz
cd SDL2_ttf-2.22.0
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_PREFIX_PATH="$PREFIX" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
cmake --build build
cmake --install build

set +x
echo "所有 aarch64 依赖构建完成，安装前缀：$PREFIX"
