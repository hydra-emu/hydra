cmake_minimum_required(VERSION 3.5)
project(hydra VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS "-g -no-pie -fno-pie -Werror=return-type -DBOOST_STACKTRACE_USE_ADDR2LINE")
set(OpenGL_GL_PREFERENCE GLVND)

find_package(Boost 1.71 REQUIRED)
find_package(Threads REQUIRED)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(fmt)

add_subdirectory("src/")
add_subdirectory("GameboyTKP/")
add_subdirectory("NESTKP/")
add_subdirectory("chip8/")
add_subdirectory("N64TKP/")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
set(PROJECT_SOURCES
    resources.qrc
    qt/main.cxx
    qt/mainwindow.cxx
    qt/screenwidget.cxx
    qt/settingswindow.cxx
    qt/shadereditor.cxx
    qt/aboutwindow.cxx
    qt/keypicker.cxx
    qt/n64_debugger.cxx
    qt/nes_ppuviewer.cxx
    src/emulator.cpp
    src/global.cxx
)

qt_add_executable(hydra
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(hydra PRIVATE TKPSrc NESTKP GameboyTKP Chip8 N64TKP
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    ${Boost_LIBRARIES} ${CMAKE_DL_LIBS}
    Threads::Threads fmt::fmt dl)

set_target_properties(hydra PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER offtkp.hydra.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(hydra)
