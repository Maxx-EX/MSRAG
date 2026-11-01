cmake_minimum_required(VERSION 3.16)
project(MSRAG C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 关键：用 pkg-config 查找 SDL2 / SDL2_ttf，避免依赖缺失的 CMake config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_ttf REQUIRED IMPORTED_TARGET sdl2_ttf)

add_library(msrag_engine_obj OBJECT src/engine.c)
target_include_directories(msrag_engine_obj PRIVATE src)
target_link_libraries(msrag_engine_obj PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_ttf)

# 可执行文件：动态链接 SDL2，RPATH=$ORIGIN 从自身所在目录找 .so
add_executable(msrag src/main.c $<TARGET_OBJECTS:msrag_engine_obj>)
target_link_libraries(msrag PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_ttf m)
set_target_properties(msrag PROPERTIES
    OUTPUT_NAME "msrag"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)

# 动态库
add_library(msrag_shared SHARED src/main.c $<TARGET_OBJECTS:msrag_engine_obj>)
target_link_libraries(msrag_shared PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_ttf m)
set_target_properties(msrag_shared PROPERTIES
    OUTPUT_NAME "msrag"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)

install(TARGETS msrag msrag_shared RUNTIME DESTINATION . LIBRARY DESTINATION .)
install(DIRECTORY assets DESTINATION . OPTIONAL)
