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
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
# TODO: don't always compile with -g
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
    "-Werror=unused-parameter"
)
string(REPLACE ";" " " WARNINGS_FLAGS "${WARNINGS}")
set(CMAKE_CXX_FLAGS "-O2 -mcrc32 -no-pie ${WARNINGS_FLAGS}")
set(OpenGL_GL_PREFERENCE GLVND)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(fmt)

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
target_include_directories(hydra PRIVATE include vendored)
target_include_directories(src PRIVATE include vendored)
target_include_directories(c8 PRIVATE include vendored)
target_include_directories(gb PRIVATE include vendored)
target_include_directories(nes PRIVATE include vendored)
target_include_directories(n64 PRIVATE include vendored)
set_target_properties(hydra PROPERTIES hydra_properties
    MACOSX_BUNDLE_GUI_IDENTIFIER offtkp.hydra.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(hydra)
