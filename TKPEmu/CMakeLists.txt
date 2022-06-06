cmake_minimum_required(VERSION 3.19)
project(TKPEmu VERSION 0.1.6)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Werror=return-type")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Werror=return-type") # -O3 may not be safe to use
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

set(CPPUNIT_FILES 
    cppunit/src/cppunit/AdditionalMessage.cpp
    cppunit/src/cppunit/Asserter.cpp
    cppunit/src/cppunit/CompilerOutputter.cpp
    cppunit/src/cppunit/DefaultProtector.cpp
    cppunit/src/cppunit/Exception.cpp
    cppunit/src/cppunit/Message.cpp
    cppunit/src/cppunit/Protector.cpp
    cppunit/src/cppunit/ProtectorChain.cpp
    cppunit/src/cppunit/SourceLine.cpp
    cppunit/src/cppunit/StringTools.cpp
    cppunit/src/cppunit/SynchronizedObject.cpp
    cppunit/src/cppunit/Test.cpp
    cppunit/src/cppunit/TestCase.cpp
    cppunit/src/cppunit/TestComposite.cpp
    cppunit/src/cppunit/TestFactoryRegistry.cpp
    cppunit/src/cppunit/TestFailure.cpp
    cppunit/src/cppunit/TestLeaf.cpp
    cppunit/src/cppunit/TestNamer.cpp
    cppunit/src/cppunit/TestPath.cpp
    cppunit/src/cppunit/TestResult.cpp
    cppunit/src/cppunit/TestResultCollector.cpp
    cppunit/src/cppunit/TestRunner.cpp
    cppunit/src/cppunit/TestSuccessListener.cpp
    cppunit/src/cppunit/TestSuite.cpp
    cppunit/src/cppunit/TestSuiteBuilderContext.cpp
    cppunit/src/cppunit/TextOutputter.cpp
    cppunit/src/cppunit/TextTestProgressListener.cpp
    cppunit/src/cppunit/TextTestRunner.cpp
    cppunit/src/cppunit/TypeInfoHelper.cpp
)
set(TKP_ENABLE_TESTING 1)
if (TKP_ENABLE_TESTING EQUAL 1)
    enable_testing()
    project(GameboyTKPTest)
    set(GBTKPTEST_FILES
        GameboyTKP/qa/test_runner.cpp
        GameboyTKP/qa/test_mooneye.cpp
    )
    add_executable(GameboyTKPTest ${GBTKPTEST_FILES} ${CPPUNIT_FILES})
    target_link_libraries(GameboyTKPTest GameboyTKP TKPSrc TKPLib ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})
    add_test(NAME GameboyTKPTest COMMAND GameboyTKPTest)
endif()