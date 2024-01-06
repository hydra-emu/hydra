include(vendored/CMakeRC.cmake)

cmake_minimum_required(VERSION 3.20)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)

project(hydra
    VERSION 0.2.0
    LANGUAGES C CXX
    HOMEPAGE_URL "https://github.com/hydra-emu/hydra"
    DESCRIPTION "A multi-system, multi-platform emulator frontend"
)

add_executable(hydra)

target_compile_features(hydra PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD 20)
set(WITH_APPS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
set(DPP_BUILD_TEST OFF)

if (WIN32)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_WINDOWS WIN32_LEAN_AND_MEAN NOMINMAX)
    set(HYDRA_WINDOWS 1)
elseif(APPLE)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_MACOS)
    set(HYDRA_MACOS 1)
elseif(ANDROID)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_ANDROID)
    set(HYDRA_ANDROID 1)
elseif(IOS)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_IOS)
    set(HYDRA_IOS 1)
elseif(EMSCRIPTEN)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_WEB)
    set(HYDRA_WEB 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_LINUX)
    set(HYDRA_LINUX 1)
elseif(UNIX)
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_FREEBSD)
    set(HYDRA_FREEBSD 1)
else()
    set(HYDRA_DEFINITIONS ${HYDRA_DEFINITIONS} HYDRA_LINUX)
    set(HYDRA_LINUX 1)
    message(STATUS "Couldn't detect platform, assuming Linux")
endif()

if(HYDRA_WINDOWS)
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} OpenGL::GL)
elseif(HYDRA_MACOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES}
        "-framework OpenGL"
    )
elseif(HYDRA_LINUX)
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} OpenGL::GL)
elseif(HYDRA_FREEBSD)
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} OpenGL::GL)
elseif(HYDRA_ANDROID)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} GLESv3 EGL OpenSLES log android)
elseif(HYDRA_IOS)
    set_property(TARGET hydra PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "hydra")
    set_property(TARGET hydra PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/data/ios-info.plist.in")
    set_target_properties(hydra PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
    set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum iOS deployment version" FORCE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO" FORCE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES}
        "-framework OpenGL"
    )
elseif(HYDRA_WEB)
    set(LONG_INT 8) # for openssl
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(hydra PROPERTIES LINK_FLAGS "-sEXPORTED_FUNCTIONS=[_main_impl,_main] -sEXPORTED_RUNTIME_METHODS=ccall -sNO_DISABLE_EXCEPTION_CATCHING -sASSERTIONS -sALLOW_MEMORY_GROWTH -s TOTAL_MEMORY=192MB -lidbfs.js -s ELIMINATE\_DUPLICATE\_FUNCTIONS=1 --shell-file ${PROJECT_SOURCE_DIR}/data/shell.html -flto -s USE_CLOSURE_COMPILER=0 --closure 0")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Thanks SkyEmu for most of these configurations
if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT /Ox /Ob2 /Oi /Ot /GT /GF /GS- /fp:fast /fp:except- /MP /sdl- /EHsc /D_SECURE_SCL=0 /D_SCL_SECURE_NO_WARNINGS /D_ITERATOR_DEBUG_LEVEL=0 /D_HAS_ITERATOR_DEBUGGING=0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT /Ox /Ob2 /Oi /Ot /GT /GF /GS- /fp:fast /fp:except- /MP /sdl- /EHsc /D_SECURE_SCL=0 /D_SCL_SECURE_NO_WARNINGS /D_ITERATOR_DEBUG_LEVEL=0 /D_HAS_ITERATOR_DEBUGGING=0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3") # TODO: it's best to strip out debug symbols for web builds
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")     # to reduce the binary size
endif()

option(BUILD_QT "Build the Qt frontend" OFF)
option(BUILD_LUA "Build with lua for script support" OFF)
option(BUILD_DISCORD_BOT "Build with discord bot support" OFF)

add_subdirectory(vendored/fmt)
add_subdirectory(vendored/argparse)
add_subdirectory(vendored/toml11)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(vendored/SDL)

set(HYDRA_INCLUDES ${HYDRA_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}/vendored/openssl/include
    vendored/openssl/include
)
add_subdirectory(vendored/openssl)
set_property(TARGET ssl PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
set_property(TARGET crypto PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} ssl crypto)

if(NOT HYDRA_WEB)
    set(CURL_USE_LIBSSH2 OFF)
    set(CURL_USE_LIBPSL OFF)
    set(CURL_USE_OPENSSL ON)
    set(CURL_DISABLE_LDAP ON)
    set(USE_LIBIDN2 OFF)
    set(CURL_ENABLE_EXPORT_TARGET OFF)
    set(BUILD_TESTING OFF)
    set(BUILD_CURL_EXE OFF)
    if (WIN32)
        set(CURL_STATIC_CRT ON)
    endif()
    add_definitions(-DCURL_STATICLIB)
    add_subdirectory(vendored/curl)
    set_property(TARGET libcurl_static PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    set(HYDRA_INCLUDES ${HYDRA_INCLUDES}
        vendored/curl/include
    )
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} libcurl_static)
endif()

if(HYDRA_LINUX OR HYDRA_FREEBSD OR HYDRA_WINDOWS)
    find_package(OpenGL REQUIRED)
endif()

if(HYDRA_MACOS OR HYDRA_IOS)
    set_target_properties(hydra PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_BUNDLE_NAME hydra
        MACOSX_BUNDLE_BUNDLE_IDENTIFIER "com.hydra.hydra_emu")
endif()

set(HYDRA_QT_FILES
    data/resources.qrc
    qt/mainwindow.cxx
    qt/screenwidget.cxx
    qt/settingswindow.cxx
    qt/shadereditor.cxx
    qt/scripteditor.cxx
    qt/aboutwindow.cxx
    qt/keypicker.cxx
    qt/terminalwindow.cxx
    qt/downloaderwindow.cxx
    qt/cheatswindow.cxx
)

set(HYDRA_IMGUI_FILES
    src/app.cxx
    src/gamewindow.cxx
    src/mainwindow.cxx
    vendored/imgui/backends/imgui_impl_sdl3.cpp
    vendored/imgui/backends/imgui_impl_opengl3.cpp
    vendored/imgui/imgui.cpp
    vendored/imgui/imgui_demo.cpp
    vendored/imgui/imgui_draw.cpp
    vendored/imgui/imgui_widgets.cpp
    vendored/imgui/imgui_tables.cpp
)

set(HYDRA_COMMON_FILES
    src/corewrapper.cxx
    src/main.cxx
    vendored/argparse/argparse.c
    vendored/miniaudio.c
    vendored/stb_image_write.c
    vendored/glad/src/glad.c
    vendored/miniz/miniz.c
    vendored/courier_prime.cxx
    vendored/material_icons.cxx
)

set(HYDRA_BOT_FILES
    discord/bot.cxx
)

set(HYDRA_INCLUDES
    ${HYDRA_INCLUDES}
    include
    core/include
    vendored
    vendored/fmt/include
    vendored/glad/include
    vendored/imgui
    discord/
    man/
)

set(HYDRA_LIBRARIES
    ${HYDRA_LIBRARIES}
    SDL3-static
    fmt
    toml11
)

cmrc_add_resource_library(hydra_images ALIAS hydra::images NAMESPACE hydra
    WHENCE data/images
    PREFIX images
    data/images/hydra.png
)

cmrc_add_resource_library(hydra_other ALIAS hydra::other NAMESPACE hydra
    WHENCE data
    PREFIX other
    data/cacert.pem
)

set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} hydra::images hydra::other)

if (BUILD_QT)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)
    add_definitions(-DHYDRA_BUILD_QT)
    set(HYDRA_SOURCES ${HYDRA_SOURCES} ${HYDRA_QT_FILES})
endif()

if (BUILD_DISCORD_BOT)
    add_subdirectory(vendored/DPP)
    add_definitions(-DHYDRA_DISCORD_BOT)
    set(HYDRA_SOURCES ${HYDRA_SOURCES} ${HYDRA_BOT_FILES})
    set(HYDRA_LIBRARIES ${HYDRA_LIBRARIES} dpp)
endif()

if (BUILD_LUA)
    find_package(Lua REQUIRED)
    set(HYDRA_INCLUDES
        ${HYDRA_INCLUDES}
        ${LUA_INCLUDE_DIR}
    )
endif()

set(HYDRA_SOURCES ${HYDRA_SOURCES} ${HYDRA_IMGUI_FILES} ${HYDRA_COMMON_FILES})

target_sources(hydra PRIVATE
    ${HYDRA_SOURCES}
)

if(BUILD_QT)
    if(HYDRA_WINDOWS)
        set_target_properties(hydra PROPERTIES COMPILE_FLAGS "/EHsc")
        target_compile_definitions(hydra PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        set_target_properties(hydra PROPERTIES WIN32_EXECUTABLE TRUE)
    elseif(HYDRA_MACOS)
        set_target_properties(hydra PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER com.hydra_emu.hydra
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE TRUE
        )
    endif()
    set(${QT_LIBS} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

target_link_libraries(hydra PRIVATE ${HYDRA_LIBRARIES})
target_include_directories(hydra PRIVATE ${HYDRA_INCLUDES})
target_compile_definitions(hydra PRIVATE HYDRA_VERSION="${PROJECT_VERSION}")
target_compile_definitions(hydra PRIVATE ${HYDRA_DEFINITIONS})

if (HYDRA_LINUX OR HYDRA_FREEBSD OR HYDRA_MACOS OR HYDRA_ANDROID OR HYDRA_WEB)
    target_compile_definitions(hydra PRIVATE HYDRA_LIBDL)
endif()