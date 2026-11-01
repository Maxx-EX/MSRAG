set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 强制 pkg-config 只搜索 aarch64 目标前缀，避免误用宿主库
set(ENV{PKG_CONFIG_LIBDIR} "/opt/aarch64-deps/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH}   "/opt/aarch64-deps/lib/pkgconfig")
