cmake_minimum_required(VERSION 3.19)
project(TKPLib)
set(FILES ../imgui/imgui.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_tables.cpp 
    ../imgui/imgui_widgets.cpp imgui_impl_opengl3.cpp imgui_impl_sdl.cpp 
    md5.cpp widget_keyselector.cpp)
add_library(TKPLib ${FILES})
target_include_directories(TKPLib PUBLIC ../)
