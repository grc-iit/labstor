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
include modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/depend.make

# Include the progress variables for this target.
include modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/progress.make

# Include the compile flags for this target's objects.
include modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/flags.make

modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.o: modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/flags.make
modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.o: ../modules/kernel/workers/netlink_client/worker_user_netlink.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.o"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.o -c /home/llogan/labstor/modules/kernel/workers/netlink_client/worker_user_netlink.cpp

modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.i"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/modules/kernel/workers/netlink_client/worker_user_netlink.cpp > CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.i

modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.s"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/modules/kernel/workers/netlink_client/worker_user_netlink.cpp -o CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.s

# Object files for target worker_client_netlink
worker_client_netlink_OBJECTS = \
"CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.o"

# External object files for target worker_client_netlink
worker_client_netlink_EXTERNAL_OBJECTS =

modules/kernel/workers/libworker_client_netlink.so: modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/netlink_client/worker_user_netlink.cpp.o
modules/kernel/workers/libworker_client_netlink.so: modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/build.make
modules/kernel/workers/libworker_client_netlink.so: liblabstor_kernel_client.so
modules/kernel/workers/libworker_client_netlink.so: modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libworker_client_netlink.so"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/worker_client_netlink.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/build: modules/kernel/workers/libworker_client_netlink.so

.PHONY : modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/build

modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/clean:
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers && $(CMAKE_COMMAND) -P CMakeFiles/worker_client_netlink.dir/cmake_clean.cmake
.PHONY : modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/clean

modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/depend:
	cd /home/llogan/labstor/cmake-build-debug-remote && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/llogan/labstor /home/llogan/labstor/modules/kernel/workers /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers /home/llogan/labstor/cmake-build-debug-remote/modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/kernel/workers/CMakeFiles/worker_client_netlink.dir/depend

