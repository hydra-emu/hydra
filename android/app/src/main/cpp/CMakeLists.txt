# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.
cmake_minimum_required(VERSION 3.22.1)
project(hydra_android)
# TODO: if anyone knows a better way lmk
set(HYDRA_BASE_DIR ../../../../..)
set(CMAKE_CXX_STANDARD 17)
add_library(${CMAKE_PROJECT_NAME} SHARED
    android.cpp ${HYDRA_BASE_DIR}/core/src/core.cxx)
target_link_libraries(${CMAKE_PROJECT_NAME}
    android
    log
    EGL
)
target_include_directories(hydra_android PRIVATE ${HYDRA_BASE_DIR}/core/include ${HYDRA_BASE_DIR}/include)