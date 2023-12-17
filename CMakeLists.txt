cmake_minimum_required(VERSION 3.12)

project(hydra
    VERSION 0.2.0
    LANGUAGES C CXX
    HOMEPAGE_URL "https://github.com/hydra-emu/hydra"
    DESCRIPTION "A multi-system, multi-platform emulator frontend"
)

option(USE_LUA "Use lua for script support" ON)

add_subdirectory(vendored/fmt)
add_subdirectory(vendored/argparse)
add_subdirectory(vendored/DPP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(OpenSSL REQUIRED)

if (USE_LUA)
    find_package(Lua REQUIRED)
    set(HYDRA_INCLUDE_DIRECTORIES
        ${HYDRA_INCLUDE_DIRECTORIES}
        ${LUA_INCLUDE_DIR}
    )
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
    src/corewrapper.cxx
    src/main.cxx
    vendored/miniaudio.c
    vendored/stb_image_write.c
    vendored/miniz/miniz.c
    vendored/argparse/argparse.c
)

set(HYDRA_BOT_FILES
    discord/bot.cxx
    vendored/gifenc.c
)

set(HYDRA_INCLUDE_DIRECTORIES
    ${HYDRA_INCLUDE_DIRECTORIES}
    include
    core/include
    vendored
    vendored/fmt/include
    qt/
    discord/
    man/
)

qt_add_executable(hydra
    MANUAL_FINALIZATION
    ${HYDRA_QT_FILES}
    ${HYDRA_BOT_FILES}
)

if(WIN32)
    set_target_properties(hydra PROPERTIES COMPILE_FLAGS "/EHsc")
    target_compile_definitions(hydra PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    set_target_properties(hydra PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    target_link_libraries(hydra PRIVATE "-framework Security")
    set_target_properties(hydra PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.hydra_emu.hydra
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
    )
endif()

target_link_libraries(hydra PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    ${CMAKE_DL_LIBS}
    ${LUA_LIBRARIES}
    OpenSSL::SSL
    fmt::fmt
    dpp
)
target_include_directories(hydra PRIVATE ${HYDRA_INCLUDE_DIRECTORIES})
target_compile_definitions(hydra PRIVATE HYDRA_VERSION="${PROJECT_VERSION}")

qt_finalize_executable(hydra)
