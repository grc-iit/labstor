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

# Include any dependencies generated for this target.
include test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/depend.make

# Include the progress variables for this target.
include test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/progress.make

# Include the compile flags for this target's objects.
include test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/flags.make

test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.o: test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/flags.make
test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.o: ../test/unit/unordered_map/client_client/test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.o"
	cd /home/llogan/labstor/cmake-build-debug-remote/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.o -c /home/llogan/labstor/test/unit/unordered_map/client_client/test.cpp

test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.i"
	cd /home/llogan/labstor/cmake-build-debug-remote/test/unit && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/test/unit/unordered_map/client_client/test.cpp > CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.i

test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.s"
	cd /home/llogan/labstor/cmake-build-debug-remote/test/unit && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/test/unit/unordered_map/client_client/test.cpp -o CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.s

# Object files for target test_shmem_unordered_map2_exec
test_shmem_unordered_map2_exec_OBJECTS = \
"CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.o"

# External object files for target test_shmem_unordered_map2_exec
test_shmem_unordered_map2_exec_EXTERNAL_OBJECTS =

test/unit/test_shmem_unordered_map2_exec: test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/unordered_map/client_client/test.cpp.o
test/unit/test_shmem_unordered_map2_exec: test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/build.make
test/unit/test_shmem_unordered_map2_exec: modules/kernel/secure_shmem/libsecure_shmem_client_netlink.so
test/unit/test_shmem_unordered_map2_exec: liblabstor_kernel_client.so
test/unit/test_shmem_unordered_map2_exec: test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test_shmem_unordered_map2_exec"
	cd /home/llogan/labstor/cmake-build-debug-remote/test/unit && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_shmem_unordered_map2_exec.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/build: test/unit/test_shmem_unordered_map2_exec

.PHONY : test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/build

test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/clean:
	cd /home/llogan/labstor/cmake-build-debug-remote/test/unit && $(CMAKE_COMMAND) -P CMakeFiles/test_shmem_unordered_map2_exec.dir/cmake_clean.cmake
.PHONY : test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/clean

test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/depend:
	cd /home/llogan/labstor/cmake-build-debug-remote && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/llogan/labstor /home/llogan/labstor/test/unit /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote/test/unit /home/llogan/labstor/cmake-build-debug-remote/test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/unit/CMakeFiles/test_shmem_unordered_map2_exec.dir/depend

