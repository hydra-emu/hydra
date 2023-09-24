cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

project(hydra VERSION 0.2.0 LANGUAGES CXX)
project(src)

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

add_subdirectory(vendored/fmt)

# add_compile_options(-fsanitize=address -fsanitize=undefined)
# add_link_options(-fsanitize=address -fsanitize=undefined)

set(QT_SOURCES
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
)

set(SRC_FILES
    src/ui_common.cxx
    src/settings.cxx
)

# ExternalProject_Add(
#     nds_rust_core
#     DOWNLOAD_COMMAND ""
#     CONFIGURE_COMMAND ""
#     INSTALL_COMMAND ""
#     BUILD_COMMAND cargo build COMMAND cargo build --release 
#     BINARY_DIR "${CMAKE_SOURCE_DIR}/nds"
#     BUILD_ALWAYS true
#     LOG_BUILD ON
# )

set(HYDRA_INCLUDE_DIRECTORIES
    include
    vendored
    vendored/fmt/include
    ${LUA_INCLUDE_DIR}
)

qt_add_executable(hydra
    MANUAL_FINALIZATION
    ${QT_SOURCES}
)

add_library(src STATIC ${SRC_FILES})

# target_link_libraries(nds
#     debug "${CMAKE_SOURCE_DIR}/nds/target/libhydra_nds.a"
#     optimized "${CMAKE_SOURCE_DIR}/nds/target/libhydra_nds.a"
# )

target_link_libraries(hydra PRIVATE src 
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets ${CMAKE_DL_LIBS}
    fmt::fmt ${LUA_LIBRARIES})
target_include_directories(hydra PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_include_directories(src PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
set_target_properties(hydra PROPERTIES hydra_properties
    MACOSX_BUNDLE_GUI_IDENTIFIER offtkp.hydra.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(hydra)

# Testing
if (CMAKE_TESTING_ENABLED)
find_package(GTest)
project(n64_qa)
add_library(alp-core STATIC
    vendored/angrylion-rdp-plus/common.h
    vendored/angrylion-rdp-plus/msg.h
    vendored/angrylion-rdp-plus/n64video.c
    vendored/angrylion-rdp-plus/n64video.h
    vendored/angrylion-rdp-plus/parallel.cpp
    vendored/angrylion-rdp-plus/parallel.h
    vendored/angrylion-rdp-plus/screen.h
    vendored/angrylion-rdp-plus/vdac.h
)
target_include_directories(alp-core PUBLIC vendored/angrylion-rdp-plus/)
target_link_libraries(alp-core PUBLIC -pthread)
add_executable(n64_qa n64/qa/n64_rdp_qa.cxx n64/core/n64_rdp.cxx n64/qa/n64_angrylion_replayer.cxx)
target_include_directories(n64_qa PRIVATE ${HYDRA_INCLUDE_DIRECTORIES} vendored/angrylion-rdp-plus/)
target_link_libraries(n64_qa PUBLIC GTest::gtest GTest::gtest_main fmt::fmt alp-core)
add_test(NAME n64_qa COMMAND n64_qa WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
