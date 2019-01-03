# CMake Options
cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE on)

set(BANG_EDITOR_DIR ${CMAKE_CURRENT_LIST_DIR}/BangEditor)
add_subdirectory("${BANG_EDITOR_DIR}")

project(UnClean3D C CXX)

macro(add_misc_files)
endmacro()

file(GLOB_RECURSE OTHER_FILES "Assets/[^.]*.frag"
                              "Assets/[^.]*.geom"
                              "Assets/[^.]*.vert"
                              "Assets/[^.]*.bushader"
                              "Assets/[^.]*.png"
                              "Assets/*.meta"
                              "Assets/[^.]*.svg"
                              "Assets/[^.]*.bmat"
                              "Assets/[^.]*.glsl"
                              "Assets/[^.]*.txt"
                              "src/[^.]*.h")
add_misc_files(${OTHER_FILES})

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(UnClean3D ${SRC_FILES})
add_dependencies(UnClean3D BangLib BangEditorLib)
target_link_libraries(UnClean3D BangLib BangEditorLib)
