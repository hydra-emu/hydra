cmake_minimum_required(VERSION 3.5)

project(hydra VERSION 0.2.0 LANGUAGES CXX)
project(src)
project(c8)
project(gb)
project(nes)
project(n64)

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
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
    vendored/miniaudio.c
)

set(SRC_FILES
    src/ui_common.cxx
    src/settings.cxx
    src/core.cxx
)

set(C8_FILES
    c8/c8_interpreter.cxx
)

set(GB_FILES
    # gb/gb_hc.cxx
    gb/gb_apu_ch.cxx
    gb/gb_apu.cxx
    gb/gb_bus.cxx
    gb/gb_cartridge.cxx
    gb/gb_cpu.cxx
    gb/gb_ppu.cxx
    gb/gb_timer.cxx
)

set(NES_FILES
    nes/nes_cpu.cxx
    nes/nes_cpubus.cxx
    nes/nes_ppu.cxx
    nes/nes_apu.cxx
    nes/NTSC_CRT/crt_core.c
    nes/NTSC_CRT/crt_ntsc.c
)

set(N64_FILES
    n64/n64_hc.cxx
    n64/core/n64_impl.cxx
    n64/core/n64_cpu.cxx
    n64/core/n64_rcp.cxx
    n64/core/n64_rsp.cxx
    n64/core/n64_rdp.cxx
    n64/core/n64_vi.cxx
    n64/core/n64_ai.cxx
)

set(HYDRA_INCLUDE_DIRECTORIES
    include
    vendored
    vendored/fmt/include
)

qt_add_executable(hydra
    MANUAL_FINALIZATION
    ${QT_SOURCES}
)

add_library(src STATIC ${SRC_FILES})
add_library(c8 STATIC ${C8_FILES})
add_library(gb STATIC ${GB_FILES})
add_library(nes STATIC ${NES_FILES})
add_library(n64 STATIC ${N64_FILES})
target_link_libraries(hydra PRIVATE src nes gb c8 n64
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets ${CMAKE_DL_LIBS}
    fmt::fmt ${LUA_LIBRARIES})
target_include_directories(hydra PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_include_directories(src PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_include_directories(c8 PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_include_directories(gb PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_include_directories(nes PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_include_directories(n64 PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
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
