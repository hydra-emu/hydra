cmake_minimum_required(VERSION 3.5)
project(TKPEmu VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS "-O3 -Werror=return-type -DBOOST_STACKTRACE_USE_ADDR2LINE")
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(Boost 1.71 REQUIRED)
find_package(Threads REQUIRED)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Multimedia)

add_subdirectory("src/")
add_subdirectory("lib/")
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
    qt/traceloggerwindow.cxx
    qt/nes_ppuviewer.cxx
    src/emulator.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TKPEmu
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TKPEmu APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TKPEmu SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TKPEmu
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TKPEmu PRIVATE TKPSrc TKPLib NESTKP GameboyTKP Chip8 N64TKP
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES}
    Qt6::Multimedia Threads::Threads)

set_target_properties(TKPEmu PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER offtkp.tkpemu.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TKPEmu)
endif()
