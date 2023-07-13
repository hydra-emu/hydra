cmake_minimum_required(VERSION 3.19)
project(n64)
set(CMAKE_CXX_STANDARD 20)
set(FILES
    n64_tkpwrapper.cxx
    miniaudio.c
    core/n64_impl.cxx
    core/n64_cpu.cxx
    core/n64_cpuinstructions.cxx
    core/n64_cpuinstructions_special.cxx
    core/n64_cpuinstructions_branch.cxx
    core/n64_cpuinstructions_float.cxx
    core/n64_cpuinstructions_loadstore.cxx
    core/n64_cpubus.cxx
    core/n64_cpuscheduler.cxx
    core/n64_cpuexceptions.cxx
    core/n64_rcp.cxx
    core/n64_rsp.cxx
    core/n64_rdp.cxx
    core/n64_rsp_su.cxx
    core/n64_rsp_vu.cxx
    core/n64_vi.cxx
    core/n64_ai.cxx
    core/n64_cpudisassemble.cxx
)
add_library(n64 ${FILES})
target_include_directories(n64 PUBLIC ../)
target_link_libraries(n64)