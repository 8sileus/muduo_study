# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_SOURCE_DIR = /home/lhx/myproject/muduo_study

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lhx/myproject/muduo_study/build

# Include any dependencies generated for this target.
include tests/CMakeFiles/Thread_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/Thread_test.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/Thread_test.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/Thread_test.dir/flags.make

tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.o: tests/CMakeFiles/Thread_test.dir/flags.make
tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.o: /home/lhx/myproject/muduo_study/tests/Thread_test.cpp
tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.o: tests/CMakeFiles/Thread_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lhx/myproject/muduo_study/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.o"
	cd /home/lhx/myproject/muduo_study/build/tests && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.o -MF CMakeFiles/Thread_test.dir/Thread_test.cpp.o.d -o CMakeFiles/Thread_test.dir/Thread_test.cpp.o -c /home/lhx/myproject/muduo_study/tests/Thread_test.cpp

tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Thread_test.dir/Thread_test.cpp.i"
	cd /home/lhx/myproject/muduo_study/build/tests && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lhx/myproject/muduo_study/tests/Thread_test.cpp > CMakeFiles/Thread_test.dir/Thread_test.cpp.i

tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Thread_test.dir/Thread_test.cpp.s"
	cd /home/lhx/myproject/muduo_study/build/tests && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lhx/myproject/muduo_study/tests/Thread_test.cpp -o CMakeFiles/Thread_test.dir/Thread_test.cpp.s

# Object files for target Thread_test
Thread_test_OBJECTS = \
"CMakeFiles/Thread_test.dir/Thread_test.cpp.o"

# External object files for target Thread_test
Thread_test_EXTERNAL_OBJECTS =

/home/lhx/myproject/muduo_study/bin/Thread_test: tests/CMakeFiles/Thread_test.dir/Thread_test.cpp.o
/home/lhx/myproject/muduo_study/bin/Thread_test: tests/CMakeFiles/Thread_test.dir/build.make
/home/lhx/myproject/muduo_study/bin/Thread_test: /home/lhx/myproject/muduo_study/lib/libmuduo_lib.so
/home/lhx/myproject/muduo_study/bin/Thread_test: tests/CMakeFiles/Thread_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lhx/myproject/muduo_study/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/lhx/myproject/muduo_study/bin/Thread_test"
	cd /home/lhx/myproject/muduo_study/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Thread_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/Thread_test.dir/build: /home/lhx/myproject/muduo_study/bin/Thread_test
.PHONY : tests/CMakeFiles/Thread_test.dir/build

tests/CMakeFiles/Thread_test.dir/clean:
	cd /home/lhx/myproject/muduo_study/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/Thread_test.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/Thread_test.dir/clean

tests/CMakeFiles/Thread_test.dir/depend:
	cd /home/lhx/myproject/muduo_study/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lhx/myproject/muduo_study /home/lhx/myproject/muduo_study/tests /home/lhx/myproject/muduo_study/build /home/lhx/myproject/muduo_study/build/tests /home/lhx/myproject/muduo_study/build/tests/CMakeFiles/Thread_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/Thread_test.dir/depend

