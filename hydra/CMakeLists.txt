cmake_minimum_required(VERSION 3.5)

project(TKPEmu VERSION 0.2.0 LANGUAGES CXX)
set(TKP_ENABLE_TESTING 1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS "-g -Werror=return-type")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost 1.71 REQUIRED)
find_package(Threads REQUIRED)
find_package(GLEW REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

add_subdirectory("src/")
add_subdirectory("lib/")
add_subdirectory("GameboyTKP/")
add_subdirectory("NESTKP/")
add_subdirectory("chip8/")
add_subdirectory("N64TKP/")

set(PROJECT_SOURCES
    resources.qrc
    qt/main.cxx
    qt/mainwindow.cxx
    qt/mainwindow.hxx
    qt/settingswindow.hxx
    qt/settingswindow.cxx
    qt/aboutwindow.hxx
    qt/aboutwindow.cxx
    qt/debuggerwindow.hxx
    qt/debuggerwindow.cxx
    qt/traceloggerwindow.hxx
    qt/traceloggerwindow.cxx
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
    ${SDL2_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} Threads::Threads)

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

# Testing
set(CPPUNIT_DIR cppunit/src/cppunit)
set(CPPUNIT_FILES 
    ${CPPUNIT_DIR}/AdditionalMessage.cpp
    ${CPPUNIT_DIR}/Asserter.cpp
    ${CPPUNIT_DIR}/CompilerOutputter.cpp
    ${CPPUNIT_DIR}/DefaultProtector.cpp
    ${CPPUNIT_DIR}/Exception.cpp
    ${CPPUNIT_DIR}/Message.cpp
    ${CPPUNIT_DIR}/Protector.cpp
    ${CPPUNIT_DIR}/ProtectorChain.cpp
    ${CPPUNIT_DIR}/SourceLine.cpp
    ${CPPUNIT_DIR}/StringTools.cpp
    ${CPPUNIT_DIR}/SynchronizedObject.cpp
    ${CPPUNIT_DIR}/Test.cpp
    ${CPPUNIT_DIR}/TestCase.cpp
    ${CPPUNIT_DIR}/TestComposite.cpp
    ${CPPUNIT_DIR}/TestFactoryRegistry.cpp
    ${CPPUNIT_DIR}/TestFailure.cpp
    ${CPPUNIT_DIR}/TestLeaf.cpp
    ${CPPUNIT_DIR}/TestNamer.cpp
    ${CPPUNIT_DIR}/TestPath.cpp
    ${CPPUNIT_DIR}/TestResult.cpp
    ${CPPUNIT_DIR}/TestResultCollector.cpp
    ${CPPUNIT_DIR}/TestRunner.cpp
    ${CPPUNIT_DIR}/TestSuccessListener.cpp
    ${CPPUNIT_DIR}/TestSuite.cpp
    ${CPPUNIT_DIR}/TestSuiteBuilderContext.cpp
    ${CPPUNIT_DIR}/TextOutputter.cpp
    ${CPPUNIT_DIR}/TextTestProgressListener.cpp
    ${CPPUNIT_DIR}/TextTestRunner.cpp
    ${CPPUNIT_DIR}/TypeInfoHelper.cpp
)
if (TKP_ENABLE_TESTING EQUAL 1)
    project(cppunit)
    add_library(cppunit ${CPPUNIT_FILES})
    enable_testing()
endif()
if (TKP_ENABLE_TESTING EQUAL 1)
    project(TestGameboy)
    set(GBTKPTEST_FILES
        GameboyTKP/qa/test_runner.cpp
        GameboyTKP/qa/test_gameboy.cpp
    )
    add_executable(TestGameboy ${GBTKPTEST_FILES})
    target_link_libraries(TestGameboy GameboyTKP TKPSrc TKPLib N64TKP NESTKP Chip8 ${SDL2_LIBRARIES} cppunit)
    add_test(NAME TestGameboy COMMAND TestGameboy)
endif()
if (TKP_ENABLE_TESTING EQUAL 1)
    project(TestEmulatorFactory)
    set(EMUFACTEST_FILES
        src/qa/test_runner.cpp
        src/qa/test_emulator_factory.cpp
        src/emulator.cpp
        src/emulator_user_data.cxx
    )
    add_executable(TestEmulatorFactory ${EMUFACTEST_FILES})
    target_link_libraries(TestEmulatorFactory TKPLib TKPSrc N64TKP NESTKP GameboyTKP Chip8 ${SDL2_LIBRARIES} cppunit)
    add_test(Name TestEmulatorFactory COMMAND TestEmulatorFactory)
endif()