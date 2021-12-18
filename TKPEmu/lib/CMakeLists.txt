cmake_minimum_required(VERSION 3.19)
project(tkp_lib)
set(IMGUI_REMOTE_ROOT https://raw.githubusercontent.com/ocornut/imgui/master)
set(IMGUI_FILES imconfig.h imgui.cpp imgui.h imgui_draw.cpp 
    imgui_internal.h imgui_tables.cpp imgui_widgets.cpp 
    imstb_rectpack.h imstb_textedit.h imstb_truetype.h)

foreach(IMGUI_FILE ${IMGUI_FILES})
    file(DOWNLOAD
        ${IMGUI_REMOTE_ROOT}/${IMGUI_FILE}
        ${PROJECT_SOURCE_DIR}/${IMGUI_FILE}
        STATUS DOWNLOAD_STATUS)
    list (GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list (GET DOWNLOAD_STATUS 0 ERROR_MESSAGE)
    if (${STATUS_CODE} EQUAL 0)
        message(STATUS "Downloaded ${IMGUI_FILE}")
    else ()
        message(FATAL_ERROR "Error occured while downloading: ${IMGUI_FILE}\n${ERROR_MESSAGE}")
    endif()
endforeach()
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(tkp_lib ${HDR_FILES} ${SRC_FILES})