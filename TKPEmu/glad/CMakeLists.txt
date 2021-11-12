cmake_minimum_required(VERSION 3.0)
project(glad)

add_library(glad glad/glad.h glad.c)
target_include_directories(glad PUBLIC include/)
