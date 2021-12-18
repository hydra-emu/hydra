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
    set(_DFILES ${DFILES} ${ARGN})
    foreach(FILE ${_DFILES})
        #if (NOT EXISTS ${PROJECT_SOURCE_DIR}/${FILE})
            DOWNLOAD_FILE(${ROOT_URL}/${FILE} ${PROJECT_SOURCE_DIR}/${FILE})
        #endif()
    endforeach()
endfunction()