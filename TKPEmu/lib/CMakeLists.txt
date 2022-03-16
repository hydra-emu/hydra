cmake_minimum_required(VERSION 3.19)
project(tkp_lib)
set(IMGUI_FILES ../imgui/imconfig.h ../imgui/imgui.cpp ../imgui/imgui.h ../imgui/imgui_draw.cpp 
    ../imgui/imgui_internal.h ../imgui/imgui_tables.cpp ../imgui/imgui_widgets.cpp 
    ../imgui/imstb_rectpack.h ../imgui/imstb_textedit.h ../imgui/imstb_truetype.h)
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(tkp_lib ${HDR_FILES} ${SRC_FILES} ${IMGUI_FILES})