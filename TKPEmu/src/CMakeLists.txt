cmake_minimum_required(VERSION 3.19)
project(tkp_src)
file(GLOB SRC_FILES "*.cpp")
add_library(tkp_src ${SRC_FILES})

