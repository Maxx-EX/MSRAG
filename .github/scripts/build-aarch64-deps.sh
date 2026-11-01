#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${1:-$(pwd)}"
PREFIX="${DEPS_PREFIX:?请设置 DEPS_PREFIX 环境变量}"
TOOLCHAIN="$PROJECT_ROOT/cmake/aarch64-linux-gnu.cmake"

export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
export CFLAGS="-I$PREFIX/include"
export LDFLAGS="-L$PREFIX/lib"

mkdir -p "$PREFIX"

wget -q https://zlib.net/zlib-1.3.1.tar.gz
tar xf zlib-1.3.1.tar.gz && cd zlib-1.3.1
CHOST=aarch64-linux-gnu ./configure --prefix="$PREFIX" --static
make -j$(nproc) && make install && cd ..

wget -q https://download.sourceforge.net/libpng/libpng-1.6.43.tar.gz
tar xf libpng-1.6.43.tar.gz && cd libpng-1.6.43
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make -j$(nproc) && make install && cd ..

wget -q https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.xz
tar xf freetype-2.13.2.tar.xz && cd freetype-2.13.2
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
  --disable-shared --enable-static --with-zlib=yes --with-png=yes --with-harfbuzz=no \
  CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make -j$(nproc) && make install && cd ..

wget -q https://www.libsdl.org/release/SDL2-2.30.8.tar.gz
tar xf SDL2-2.30.8.tar.gz && cd SDL2-2.30.8
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_PREFIX_PATH="$PREFIX" \
  -DSDL_STATIC=ON -DSDL_SHARED=ON \
  -DSDL_TEST=OFF -DSDL_TESTS=OFF
cmake --build build -j$(nproc) && cmake --install build && cd ..

wget -q https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.22.0.tar.gz
tar xf SDL2_ttf-2.22.0.tar.gz && cd SDL2_ttf-2.22.0
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_PREFIX_PATH="$PREFIX" \
  -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
cmake --build build -j$(nproc) && cmake --install build
