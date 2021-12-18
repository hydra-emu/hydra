cmake_minimum_required(VERSION 3.19)
project(tkp_lib)
set(imgui_remote_root https://github.com/ocornut/imgui/raw/master/)
#file(DOWNLOAD
#    https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/src/vk_mem_alloc.h
#    ${CMAKE_SOURCE_DIR}/external/VulkanMemoryAllocator/vk_mem_alloc.h)
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(tkp_lib ${HDR_FILES} ${SRC_FILES})