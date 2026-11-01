cmake_minimum_required(VERSION 3.16)
project(MSRAG C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_library(msrag_engine_obj OBJECT src/engine.c)
target_include_directories(msrag_engine_obj PRIVATE src)

# 静态可执行文件：SDL2 静态链接
add_executable(msrag src/main.c $<TARGET_OBJECTS:msrag_engine_obj>)
target_include_directories(msrag PRIVATE src)
target_link_libraries(msrag PRIVATE SDL2::SDL2-static SDL2_ttf::SDL2_ttf-static m)
set_target_properties(msrag PROPERTIES
    OUTPUT_NAME "msrag"
    LINK_FLAGS "-static-libgcc"
)

# 动态库：动态链接 SDL2，打包时附带 .so
add_library(msrag_shared SHARED src/main.c $<TARGET_OBJECTS:msrag_engine_obj>)
target_include_directories(msrag_shared PRIVATE src)
target_link_libraries(msrag_shared PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf m)
set_target_properties(msrag_shared PROPERTIES
    OUTPUT_NAME "msrag"
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
)

install(TARGETS msrag msrag_shared RUNTIME DESTINATION . LIBRARY DESTINATION .)

# 仅在原生构建时复制系统 SDL2 .so；交叉编译时由 workflow 另行处理
if(NOT CMAKE_CROSSCOMPILING)
    install(FILES
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_SONAME_FILE:SDL2::SDL2>
        $<TARGET_FILE:SDL2_ttf::SDL2_ttf>
        $<TARGET_SONAME_FILE:SDL2_ttf::SDL2_ttf>
        DESTINATION .
    )
endif()

install(DIRECTORY assets DESTINATION . OPTIONAL)
