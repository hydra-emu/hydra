cmake_minimum_required(VERSION 3.19)
project(tkp_lib)
include(../TKPDownload.cmake)
# If the download directory for ImGui ever changes, change the following URL
set(IMGUI_REMOTE_ROOT https://github.com/ocornut/imgui.git)
set(IMGUI_REMOTE_DIR https://raw.githubusercontent.com/ocornut/imgui/master)
set(IMGUI_FILES imconfig.h imgui.cpp imgui.h imgui_draw.cpp 
    imgui_internal.h imgui_tables.cpp imgui_widgets.cpp 
    imstb_rectpack.h imstb_textedit.h imstb_truetype.h)
DOWNLOAD_FILES(${IMGUI_REMOTE_ROOT} ${IMGUI_REMOTE_DIR} ${IMGUI_FILES})
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(tkp_lib ${HDR_FILES} ${SRC_FILES})