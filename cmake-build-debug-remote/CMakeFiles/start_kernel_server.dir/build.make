# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/llogan/labstor

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/llogan/labstor/cmake-build-debug-remote

# Utility rule file for start_kernel_server.

# Include the progress variables for this target.
include CMakeFiles/start_kernel_server.dir/progress.make

CMakeFiles/start_kernel_server:
	LABSTOR_ROOT=/home/llogan/labstor LABSTOR_BIN=/home/llogan/labstor/cmake-build-debug-remote bash /home/llogan/labstor/util/start_kernel_server.sh

start_kernel_server: CMakeFiles/start_kernel_server
start_kernel_server: CMakeFiles/start_kernel_server.dir/build.make

.PHONY : start_kernel_server

# Rule to build all files generated by this target.
CMakeFiles/start_kernel_server.dir/build: start_kernel_server

.PHONY : CMakeFiles/start_kernel_server.dir/build

CMakeFiles/start_kernel_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/start_kernel_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/start_kernel_server.dir/clean

CMakeFiles/start_kernel_server.dir/depend:
	cd /home/llogan/labstor/cmake-build-debug-remote && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/llogan/labstor /home/llogan/labstor /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote/CMakeFiles/start_kernel_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/start_kernel_server.dir/depend

