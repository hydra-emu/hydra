cmake_minimum_required(VERSION 3.19)
project(tkp_lib)
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(tkp_lib ${HDR_FILES} ${SRC_FILES})

