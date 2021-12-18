cmake_minimum_required(VERSION 3.19)
project(tkp_lib)
# DOWNLOAD_FILE function - OFFTKP
# Downloads a file from ROOT_FILE and stores it in LOCAL_FILE
# without checking if it exists already
function(DOWNLOAD_FILE ROOT_FILE LOCAL_FILE)
    file(DOWNLOAD
        ${ROOT_FILE}
        ${LOCAL_FILE}
        STATUS DOWNLOAD_STATUS)
    file(SHA1 ${PROJECT_SOURCE_DIR}/${FILE} HASH)
    
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 0 ERROR_MESSAGE)
    if (${STATUS_CODE} EQUAL 0)
        message(STATUS "Downloaded ${FILE} - ${HASH}")
    else ()
        message(FATAL_ERROR "Error occured while downloading: ${FILE}")
    endif()
endfunction()
# Downloads multiple files after checking if they exist already
function(DOWNLOAD_FILES ROOT_URL DFILES)
    foreach(FILE ${DFILES})
        if (NOT EXISTS ${PROJECT_SOURCE_DIR}/${FILE})
            DOWNLOAD_FILE(${ROOT_URL}/${FILE} ${PROJECT_SOURCE_DIR}/${FILE})
        endif()
    endforeach()
endfunction()
# If the download directory for ImGui ever changes, change the following URL
set(IMGUI_REMOTE_ROOT https://raw.githubusercontent.com/ocornut/imgui/master)
set(IMGUI_FILES imconfig.h imgui.cpp imgui.h imgui_draw.cpp 
    imgui_internal.h imgui_tables.cpp imgui_widgets.cpp 
    imstb_rectpack.h imstb_textedit.h imstb_truetype.h)
DOWNLOAD_FILES(${IMGUI_REMOTE_ROOT} ${IMGUI_FILES})
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(tkp_lib ${HDR_FILES} ${SRC_FILES})