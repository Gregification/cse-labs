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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/greg/Public/cse-labs/cse3320/lab3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/greg/Public/cse-labs/cse3320/lab3/uild

# Include any dependencies generated for this target.
include CMakeFiles/Assignemnt3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Assignemnt3.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Assignemnt3.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Assignemnt3.dir/flags.make

CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o: CMakeFiles/Assignemnt3.dir/flags.make
CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o: /home/greg/Public/cse-labs/cse3320/lab3/src/FAT.cpp
CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o: CMakeFiles/Assignemnt3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/greg/Public/cse-labs/cse3320/lab3/uild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o -MF CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o.d -o CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o -c /home/greg/Public/cse-labs/cse3320/lab3/src/FAT.cpp

CMakeFiles/Assignemnt3.dir/src/FAT.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Assignemnt3.dir/src/FAT.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/greg/Public/cse-labs/cse3320/lab3/src/FAT.cpp > CMakeFiles/Assignemnt3.dir/src/FAT.cpp.i

CMakeFiles/Assignemnt3.dir/src/FAT.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Assignemnt3.dir/src/FAT.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/greg/Public/cse-labs/cse3320/lab3/src/FAT.cpp -o CMakeFiles/Assignemnt3.dir/src/FAT.cpp.s

CMakeFiles/Assignemnt3.dir/src/main.cpp.o: CMakeFiles/Assignemnt3.dir/flags.make
CMakeFiles/Assignemnt3.dir/src/main.cpp.o: /home/greg/Public/cse-labs/cse3320/lab3/src/main.cpp
CMakeFiles/Assignemnt3.dir/src/main.cpp.o: CMakeFiles/Assignemnt3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/greg/Public/cse-labs/cse3320/lab3/uild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Assignemnt3.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Assignemnt3.dir/src/main.cpp.o -MF CMakeFiles/Assignemnt3.dir/src/main.cpp.o.d -o CMakeFiles/Assignemnt3.dir/src/main.cpp.o -c /home/greg/Public/cse-labs/cse3320/lab3/src/main.cpp

CMakeFiles/Assignemnt3.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Assignemnt3.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/greg/Public/cse-labs/cse3320/lab3/src/main.cpp > CMakeFiles/Assignemnt3.dir/src/main.cpp.i

CMakeFiles/Assignemnt3.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Assignemnt3.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/greg/Public/cse-labs/cse3320/lab3/src/main.cpp -o CMakeFiles/Assignemnt3.dir/src/main.cpp.s

# Object files for target Assignemnt3
Assignemnt3_OBJECTS = \
"CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o" \
"CMakeFiles/Assignemnt3.dir/src/main.cpp.o"

# External object files for target Assignemnt3
Assignemnt3_EXTERNAL_OBJECTS =

Assignemnt3: CMakeFiles/Assignemnt3.dir/src/FAT.cpp.o
Assignemnt3: CMakeFiles/Assignemnt3.dir/src/main.cpp.o
Assignemnt3: CMakeFiles/Assignemnt3.dir/build.make
Assignemnt3: CMakeFiles/Assignemnt3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/greg/Public/cse-labs/cse3320/lab3/uild/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable Assignemnt3"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Assignemnt3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Assignemnt3.dir/build: Assignemnt3
.PHONY : CMakeFiles/Assignemnt3.dir/build

CMakeFiles/Assignemnt3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Assignemnt3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Assignemnt3.dir/clean

CMakeFiles/Assignemnt3.dir/depend:
	cd /home/greg/Public/cse-labs/cse3320/lab3/uild && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/greg/Public/cse-labs/cse3320/lab3 /home/greg/Public/cse-labs/cse3320/lab3 /home/greg/Public/cse-labs/cse3320/lab3/uild /home/greg/Public/cse-labs/cse3320/lab3/uild /home/greg/Public/cse-labs/cse3320/lab3/uild/CMakeFiles/Assignemnt3.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/Assignemnt3.dir/depend
