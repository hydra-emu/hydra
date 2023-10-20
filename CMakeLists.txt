cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

project(hydra VERSION 0.2.0 LANGUAGES CXX)
project(hydra_server)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++20")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcrc32")
    endif()
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
    endif()
elseif(LINUX)
    set(WARNINGS 
        "-Werror=return-type"
        "-Werror=unused-variable"
        "-Werror=unused-but-set-variable"
        "-Werror=unused-function"
        "-Werror=uninitialized"
        "-Wimplicit-fallthrough"
        "-Werror=sign-compare"
        "-Werror=shadow"
        "-Werror=deprecated-declarations"
    )
    string(REPLACE ";" " " WARNINGS_FLAGS "${WARNINGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcrc32 ${WARNINGS_FLAGS} -g -O2")
elseif(EMSCRIPTEN)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
set(OpenGL_GL_PREFERENCE GLVND)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(Lua REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(vendored/fmt)
add_subdirectory(vendored/argparse)

# add_compile_options(-fsanitize=address -fsanitize=undefined)
# add_link_options(-fsanitize=address -fsanitize=undefined)

set(HYDRA_COMMON_FILES
    core/src/core.cxx
)

set(HYDRA_QT_FILES
    data/resources.qrc
    qt/main.cxx
    qt/mainwindow.cxx
    qt/screenwidget.cxx
    qt/settingswindow.cxx
    qt/shadereditor.cxx
    qt/scripteditor.cxx
    qt/aboutwindow.cxx
    qt/keypicker.cxx
    qt/terminalwindow.cxx
    vendored/miniaudio.c
    vendored/stb_image_write.c
)

set(HYDRA_SERVER_FILES
    server/main.cxx
    server/server.cxx
    vendored/glad.c
)

set(HYDRA_INCLUDE_DIRECTORIES
    include
    core/include
    vendored
    vendored/fmt/include
    ${LUA_INCLUDE_DIR}
)

qt_add_executable(hydra
    MANUAL_FINALIZATION
    ${HYDRA_COMMON_FILES}
    ${HYDRA_QT_FILES}
)

target_link_libraries(hydra PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets ${CMAKE_DL_LIBS}
    fmt::fmt ${LUA_LIBRARIES})
target_include_directories(hydra PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
set_target_properties(hydra PROPERTIES hydra_properties
    MACOSX_BUNDLE_GUI_IDENTIFIER offtkp.hydra.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(hydra)

# Server
add_executable(
    hydra_server
    ${HYDRA_COMMON_FILES}
    ${HYDRA_SERVER_FILES}
)
target_include_directories(hydra_server PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_link_libraries(hydra_server PRIVATE argparse glfw)
if (NOT BUILD_SERVER)
   set_target_properties(hydra_server PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()
