cmake_minimum_required(VERSION 3.5)
project(n64_hydra)
set(N64_FILES
    core.cxx
    core/n64_impl.cxx
    core/n64_cpu.cxx
    core/n64_rcp.cxx
    core/n64_rsp.cxx
    core/n64_rdp.cxx
    core/n64_vi.cxx
    core/n64_ai.cxx
)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_library(n64_hydra SHARED ${N64_FILES})