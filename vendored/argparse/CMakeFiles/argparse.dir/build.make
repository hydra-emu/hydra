# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/runner/work/hydra/hydra

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/runner/work/hydra/hydra/build

# Include any dependencies generated for this target.
include vendored/argparse/CMakeFiles/argparse.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include vendored/argparse/CMakeFiles/argparse.dir/compiler_depend.make

# Include the progress variables for this target.
include vendored/argparse/CMakeFiles/argparse.dir/progress.make

# Include the compile flags for this target's objects.
include vendored/argparse/CMakeFiles/argparse.dir/flags.make

vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o: vendored/argparse/CMakeFiles/argparse.dir/flags.make
vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o: vendored/argparse/CMakeFiles/argparse.dir/includes_C.rsp
vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o: /home/runner/work/hydra/hydra/vendored/argparse/argparse.c
vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o: vendored/argparse/CMakeFiles/argparse.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o"
	cd /home/runner/work/hydra/hydra/build/vendored/argparse && /home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/emcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o -MF CMakeFiles/argparse.dir/argparse.c.o.d -o CMakeFiles/argparse.dir/argparse.c.o -c /home/runner/work/hydra/hydra/vendored/argparse/argparse.c

vendored/argparse/CMakeFiles/argparse.dir/argparse.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/argparse.dir/argparse.c.i"
	cd /home/runner/work/hydra/hydra/build/vendored/argparse && /home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/emcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/runner/work/hydra/hydra/vendored/argparse/argparse.c > CMakeFiles/argparse.dir/argparse.c.i

vendored/argparse/CMakeFiles/argparse.dir/argparse.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/argparse.dir/argparse.c.s"
	cd /home/runner/work/hydra/hydra/build/vendored/argparse && /home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/emcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/runner/work/hydra/hydra/vendored/argparse/argparse.c -o CMakeFiles/argparse.dir/argparse.c.s

# Object files for target argparse
argparse_OBJECTS = \
"CMakeFiles/argparse.dir/argparse.c.o"

# External object files for target argparse
argparse_EXTERNAL_OBJECTS =

vendored/argparse/libargparse.a: vendored/argparse/CMakeFiles/argparse.dir/argparse.c.o
vendored/argparse/libargparse.a: vendored/argparse/CMakeFiles/argparse.dir/build.make
vendored/argparse/libargparse.a: vendored/argparse/CMakeFiles/argparse.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libargparse.a"
	cd /home/runner/work/hydra/hydra/build/vendored/argparse && $(CMAKE_COMMAND) -P CMakeFiles/argparse.dir/cmake_clean_target.cmake
	cd /home/runner/work/hydra/hydra/build/vendored/argparse && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/argparse.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
vendored/argparse/CMakeFiles/argparse.dir/build: vendored/argparse/libargparse.a
.PHONY : vendored/argparse/CMakeFiles/argparse.dir/build

vendored/argparse/CMakeFiles/argparse.dir/clean:
	cd /home/runner/work/hydra/hydra/build/vendored/argparse && $(CMAKE_COMMAND) -P CMakeFiles/argparse.dir/cmake_clean.cmake
.PHONY : vendored/argparse/CMakeFiles/argparse.dir/clean

vendored/argparse/CMakeFiles/argparse.dir/depend:
	cd /home/runner/work/hydra/hydra/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/runner/work/hydra/hydra /home/runner/work/hydra/hydra/vendored/argparse /home/runner/work/hydra/hydra/build /home/runner/work/hydra/hydra/build/vendored/argparse /home/runner/work/hydra/hydra/build/vendored/argparse/CMakeFiles/argparse.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : vendored/argparse/CMakeFiles/argparse.dir/depend
