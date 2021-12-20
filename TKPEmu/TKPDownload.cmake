# DOWNLOAD_FILE function - OFFTKP
# Downloads a file from ROOT_DIR and stores it in LOCAL_FILE
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
# Downloads multiple files after checking if there has been any changes
# If you need to force redownload, remove previous_version.cache from the respective folder
function(DOWNLOAD_FILES ROOT_GIT ROOT_URL DFILES)
    find_package(Git)
    if (NOT GIT_FOUND)
        message("ERROR: Git not found!")
    endif()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} ls-remote
        ${ROOT_GIT} HEAD
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output)
    set(old_cache)
    if (EXISTS "${PROJECT_SOURCE_DIR}/previous_version.cache")
        file(READ "${PROJECT_SOURCE_DIR}/previous_version.cache" old_cache)
    else()
        set(old_cache "")
    endif()
    file(WRITE "${PROJECT_SOURCE_DIR}/previous_version.cache" "${output}")
    if (NOT "${old_cache}" STREQUAL "${output}")
        message("New commits found, redownloading...")
        message("${old_cache} ${output}")
        set(_DFILES ${DFILES} ${ARGN})
        foreach(FILE ${_DFILES})
            DOWNLOAD_FILE(${ROOT_URL}/${FILE} ${PROJECT_SOURCE_DIR}/${FILE})
        endforeach()
    endif()
endfunction()