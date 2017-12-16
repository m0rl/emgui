cmake_minimum_required(VERSION 3.5)
project(emgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(emgui STATIC src/gles_device.cpp src/window_manager.cpp)
target_include_directories(emgui PUBLIC include/)
target_compile_options(emgui PRIVATE -Wall -pedantic -Werror)

add_subdirectory(vendor/imgui)
target_link_libraries(emgui PUBLIC imgui)

add_subdirectory(example)
