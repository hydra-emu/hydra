cmake_minimum_required(VERSION 3.19)
project(TKPEmu VERSION 0.1.6)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Werror=return-type")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Werror=return-type") # -O3 may not be safe to use
add_subdirectory("src/")
add_subdirectory("lib/")
add_subdirectory("GameboyTKP/")
add_subdirectory("NESTKP/")
add_subdirectory("chip8/")
add_subdirectory("N64TKP/")
# find_package(Lua REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost 1.71 REQUIRED)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)
find_package(GLEW REQUIRED)
add_executable(TKPEmu main.cpp)
target_include_directories(TKPEmu PUBLIC ./)
target_link_libraries(TKPEmu PRIVATE TKPSrc TKPLib NESTKP GameboyTKP Chip8
    N64TKP ${SDL2_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS}
    ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${LUA_LIBRARIES} TBB::tbb Threads::Threads)

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
set(TKP_ENABLE_TESTING 1)
if (TKP_ENABLE_TESTING EQUAL 1)
    project(cppunit)
    add_library(cppunit ${CPPUNIT_FILES})
    enable_testing()
    project(GameboyTKPTest)
    set(GBTKPTEST_FILES
        GameboyTKP/qa/test_runner.cpp
        GameboyTKP/qa/test_mooneye.cpp
    )
    add_executable(GameboyTKPTest ${GBTKPTEST_FILES})
    target_link_libraries(GameboyTKPTest GameboyTKP TKPSrc TKPLib ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} cppunit)
    add_test(NAME GameboyTKPTest COMMAND GameboyTKPTest)
endif()