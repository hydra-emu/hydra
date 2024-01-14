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
include CMakeFiles/hydra_data.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/hydra_data.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/hydra_data.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/hydra_data.dir/flags.make

__cmrc_hydra_data/lib.cpp: __cmrc_hydra_data/lib_.cpp
__cmrc_hydra_data/lib.cpp: _cmrc/include/cmrc/cmrc.hpp
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating hydra_data resource loader"
	/usr/local/bin/cmake -E copy_if_different /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/lib_.cpp /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/lib.cpp

__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp: /home/runner/work/hydra/hydra/data/images/hydra.png
__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp: /home/runner/work/hydra/hydra/vendored/CMakeRC.cmake
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating intermediate file for /home/runner/work/hydra/hydra/data/images/hydra.png"
	/usr/local/bin/cmake -D_CMRC_GENERATE_MODE=TRUE -DNAMESPACE=hydra -DSYMBOL=f_7da7_data_images_hydra_png -DINPUT_FILE=/home/runner/work/hydra/hydra/data/images/hydra.png -DOUTPUT_FILE=/home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp -P /home/runner/work/hydra/hydra/vendored/CMakeRC.cmake

__cmrc_hydra_data/intermediate/data/cacert.pem.cpp: /home/runner/work/hydra/hydra/data/cacert.pem
__cmrc_hydra_data/intermediate/data/cacert.pem.cpp: /home/runner/work/hydra/hydra/vendored/CMakeRC.cmake
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Generating intermediate file for /home/runner/work/hydra/hydra/data/cacert.pem"
	/usr/local/bin/cmake -D_CMRC_GENERATE_MODE=TRUE -DNAMESPACE=hydra -DSYMBOL=f_f6f8_data_cacert_pem -DINPUT_FILE=/home/runner/work/hydra/hydra/data/cacert.pem -DOUTPUT_FILE=/home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp -P /home/runner/work/hydra/hydra/vendored/CMakeRC.cmake

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o: CMakeFiles/hydra_data.dir/flags.make
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o: CMakeFiles/hydra_data.dir/includes_CXX.rsp
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o: __cmrc_hydra_data/lib.cpp
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o: CMakeFiles/hydra_data.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o -MF CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o.d -o CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o -c /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/lib.cpp

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.i"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/lib.cpp > CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.i

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.s"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/lib.cpp -o CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.s

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o: CMakeFiles/hydra_data.dir/flags.make
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o: CMakeFiles/hydra_data.dir/includes_CXX.rsp
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o: __cmrc_hydra_data/intermediate/data/images/hydra.png.cpp
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o: CMakeFiles/hydra_data.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o -MF CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o.d -o CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o -c /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.i"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp > CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.i

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.s"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp -o CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.s

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o: CMakeFiles/hydra_data.dir/flags.make
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o: CMakeFiles/hydra_data.dir/includes_CXX.rsp
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o: __cmrc_hydra_data/intermediate/data/cacert.pem.cpp
CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o: CMakeFiles/hydra_data.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o -MF CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o.d -o CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o -c /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.i"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp > CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.i

CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.s"
	/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/em++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/runner/work/hydra/hydra/build/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp -o CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.s

# Object files for target hydra_data
hydra_data_OBJECTS = \
"CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o" \
"CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o" \
"CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o"

# External object files for target hydra_data
hydra_data_EXTERNAL_OBJECTS =

libhydra_data.a: CMakeFiles/hydra_data.dir/__cmrc_hydra_data/lib.cpp.o
libhydra_data.a: CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/images/hydra.png.cpp.o
libhydra_data.a: CMakeFiles/hydra_data.dir/__cmrc_hydra_data/intermediate/data/cacert.pem.cpp.o
libhydra_data.a: CMakeFiles/hydra_data.dir/build.make
libhydra_data.a: CMakeFiles/hydra_data.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/runner/work/hydra/hydra/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX static library libhydra_data.a"
	$(CMAKE_COMMAND) -P CMakeFiles/hydra_data.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hydra_data.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/hydra_data.dir/build: libhydra_data.a
.PHONY : CMakeFiles/hydra_data.dir/build

CMakeFiles/hydra_data.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/hydra_data.dir/cmake_clean.cmake
.PHONY : CMakeFiles/hydra_data.dir/clean

CMakeFiles/hydra_data.dir/depend: __cmrc_hydra_data/intermediate/data/cacert.pem.cpp
CMakeFiles/hydra_data.dir/depend: __cmrc_hydra_data/intermediate/data/images/hydra.png.cpp
CMakeFiles/hydra_data.dir/depend: __cmrc_hydra_data/lib.cpp
	cd /home/runner/work/hydra/hydra/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/runner/work/hydra/hydra /home/runner/work/hydra/hydra /home/runner/work/hydra/hydra/build /home/runner/work/hydra/hydra/build /home/runner/work/hydra/hydra/build/CMakeFiles/hydra_data.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/hydra_data.dir/depend
