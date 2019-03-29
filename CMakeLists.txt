# CMake Options
cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE on)

set(BANG_EDITOR_DIR ${CMAKE_CURRENT_LIST_DIR}/BangEditor)
set(BANG_PROJECT_ROOT ${BANG_EDITOR_DIR}/Bang)
add_subdirectory("${BANG_EDITOR_DIR}")

project(UnClean3D C CXX)

file(GLOB_RECURSE OTHER_FILES "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.frag"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.geom"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.vert"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/*.bushader"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.png"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/*.meta"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.svg"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.bmat"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.glsl"
                              "${CMAKE_CURRENT_LIST_DIR}/Assets/[^.]*.txt"
                              "${CMAKE_CURRENT_LIST_DIR}/src/[^.]*.h")
add_misc_files("${OTHER_FILES}")

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(UnClean3D ${SRC_FILES} ${OTHER_FILES})
add_dependencies(UnClean3D BangLib BangEditorLib)
add_bang_compilation_flags(UnClean3D)
target_link_libraries(UnClean3D BangLib BangEditorLib)
