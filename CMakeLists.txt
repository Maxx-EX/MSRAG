cmake_minimum_required(VERSION 3.16)
project(MSRAG C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)

# 注意：pkg-config 模块名必须是 SDL2_ttf（大写），不是 sdl2_ttf
pkg_check_modules(SDL2     REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_ttf REQUIRED IMPORTED_TARGET SDL2_ttf)

add_library(msrag_engine_obj OBJECT src/engine.c)
target_include_directories(msrag_engine_obj PRIVATE src)
target_link_libraries(msrag_engine_obj PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_ttf)

add_executable(msrag src/main.c $<TARGET_OBJECTS:msrag_engine_obj>)
target_link_libraries(msrag PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_ttf m)
set_target_properties(msrag PROPERTIES
    OUTPUT_NAME "msrag"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)

add_library(msrag_shared SHARED src/main.c $<TARGET_OBJECTS:msrag_engine_obj>)
target_link_libraries(msrag_shared PRIVATE PkgConfig::SDL2 PkgConfig::SDL2_ttf m)
set_target_properties(msrag_shared PROPERTIES
    OUTPUT_NAME "msrag"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN"
)

install(TARGETS msrag msrag_shared RUNTIME DESTINATION . LIBRARY DESTINATION .)
install(DIRECTORY assets DESTINATION . OPTIONAL)
