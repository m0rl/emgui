project(imgui)

add_library(imgui STATIC imgui/imgui.cpp imgui/imgui_draw.cpp)
target_include_directories(imgui PUBLIC imgui/)
