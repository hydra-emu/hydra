cmake_minimum_required(VERSION 3.5)

project(hydra VERSION 0.2.0 LANGUAGES CXX)
project(src)
project(c8)
project(gb)
project(nes)
project(n64)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
# TODO: don't always compile with -g
# TODO: compiler based
if (LINUX)
    set(WARNINGS 
        "-Wno-stringop-overflow" # for some fmt warning
        "-Werror=return-type"
        "-Werror=unused-variable"
        "-Werror=unused-but-set-variable"
        "-Werror=unused-function"
        "-Werror=uninitialized"
        "-Werror=implicit-fallthrough"
        "-Werror=sign-compare"
        "-Werror=shadow"
        "-Werror=deprecated-declarations"
    )
endif()
string(REPLACE ";" " " WARNINGS_FLAGS "${WARNINGS}")
set(CMAKE_CXX_FLAGS "-g -O2")
set(CMAKE_C_FLAGS "-g -O0")
if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcrc32 -no-pie ${WARNINGS_FLAGS}")
endif()
set(OpenGL_GL_PREFERENCE GLVND)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

set(PROJECT_SOURCES
    data/resources.qrc
    qt/main.cxx
    qt/mainwindow.cxx
    qt/screenwidget.cxx
    qt/settingswindow.cxx
    qt/shadereditor.cxx
    qt/aboutwindow.cxx
    qt/keypicker.cxx
    qt/n64_debugger.cxx
    qt/nes_ppuviewer.cxx
    qt/mmioviewer.cxx
    src/emulator.cxx
    src/global.cxx
)

set(SRC_FILES
    src/emulator.cxx
    src/emulator_factory.cxx
    src/emulator_user_data.cxx
    src/emulator_settings.cxx
)

set(C8_FILES
    c8/c8_interpreter.cxx
    c8/c8_tkpwrapper.cxx
)

set(GB_FILES
    gb/gb_tkpwrapper.cxx
    gb/gb_apu_ch.cxx
    gb/gb_apu.cxx
    gb/gb_bus.cxx
    gb/gb_cartridge.cxx
    gb/gb_cpu.cxx
    gb/gb_ppu.cxx
    gb/gb_timer.cxx
)

set(NES_FILES
    nes/nes_tkpwrapper.cxx
    nes/nes_cpu.cxx
    nes/nes_cpubus.cxx
    nes/nes_ppu.cxx
    nes/nes_apu.cxx
    nes/NTSC_CRT/crt_core.c
    nes/NTSC_CRT/crt_ntsc.c
)

set(N64_FILES
    n64/n64_tkpwrapper.cxx
    n64/core/n64_impl.cxx
    n64/core/n64_cpu.cxx
    n64/core/n64_cpuinstructions.cxx
    n64/core/n64_cpuinstructions_special.cxx
    n64/core/n64_cpuinstructions_branch.cxx
    n64/core/n64_cpuinstructions_float.cxx
    n64/core/n64_cpuinstructions_loadstore.cxx
    n64/core/n64_cpubus.cxx
    n64/core/n64_cpuexceptions.cxx
    n64/core/n64_rcp.cxx
    n64/core/n64_rsp.cxx
    n64/core/n64_rdp.cxx
    n64/core/n64_rsp_su.cxx
    n64/core/n64_rsp_vu.cxx
    n64/core/n64_vi.cxx
    n64/core/n64_ai.cxx
    n64/core/n64_cpudisassemble.cxx
    vendored/miniaudio.c
)

set(HYDRA_INCLUDE_DIRECTORIES
    include
    vendored
)

if (APPLE)
    include_directories(/usr/local/include /opt/homebrew/include)
endif()

qt_add_executable(hydra
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

add_library(src STATIC ${SRC_FILES})
add_library(c8 STATIC ${C8_FILES})
add_library(gb STATIC ${GB_FILES})
add_library(nes STATIC ${NES_FILES})
add_library(n64 STATIC ${N64_FILES})
target_link_libraries(hydra PRIVATE src nes gb c8 n64
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets ${CMAKE_DL_LIBS}
    fmt::fmt dl)
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

enable_testing()
project(n64_qa)
find_package(GTest REQUIRED)
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