#!/usr/bin/env bash
set -euo pipefail

PREFIX=/opt/aarch64-deps
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
export CFLAGS="-I$PREFIX/include"
export LDFLAGS="-L$PREFIX/lib"

mkdir -p "$PREFIX"

# zlib
wget -q https://zlib.net/zlib-1.3.1.tar.gz
tar xf zlib-1.3.1.tar.gz && cd zlib-1.3.1
CHOST=aarch64-linux-gnu ./configure --prefix="$PREFIX" --static
make -j$(nproc) && make install
cd ..

# libpng
wget -q https://download.sourceforge.net/libpng/libpng-1.6.43.tar.gz
tar xf libpng-1.6.43.tar.gz && cd libpng-1.6.43
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
    --disable-shared --enable-static CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make -j$(nproc) && make install
cd ..

# freetype（禁用 harfbuzz 可选，简化依赖）
wget -q https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.xz
tar xf freetype-2.13.2.tar.xz && cd freetype-2.13.2
./configure --host=aarch64-linux-gnu --prefix="$PREFIX" \
    --disable-shared --enable-static \
    --with-zlib=yes --with-png=yes --with-harfbuzz=no \
    CPPFLAGS="-I$PREFIX/include" LDFLAGS="-L$PREFIX/lib"
make -j$(nproc) && make install
cd ..

# SDL2：同时构建静态库和动态库，便于 bin 与 so 分别使用
wget -q https://www.libsdl.org/release/SDL2-2.30.8.tar.gz
tar xf SDL2-2.30.8.tar.gz && cd SDL2-2.30.8
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/workspace/cmake/aarch64-linux-gnu.cmake \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DSDL_STATIC=ON -DSDL_SHARED=ON \
    -DSDL_TEST=OFF
cmake --build build -j$(nproc) && cmake --install build
cd ..

# SDL2_ttf：静态链接 freetype，避免运行时额外 .so 依赖
wget -q https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.22.0.tar.gz
tar xf SDL2_ttf-2.22.0.tar.gz && cd SDL2_ttf-2.22.0
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/workspace/cmake/aarch64-linux-gnu.cmake \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DSDL2TTF_SDL2=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
cmake --build build -j$(nproc) && cmake --install build
