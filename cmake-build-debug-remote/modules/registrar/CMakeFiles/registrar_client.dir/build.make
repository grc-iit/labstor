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
include modules/registrar/CMakeFiles/registrar_client.dir/depend.make

# Include the progress variables for this target.
include modules/registrar/CMakeFiles/registrar_client.dir/progress.make

# Include the compile flags for this target's objects.
include modules/registrar/CMakeFiles/registrar_client.dir/flags.make

modules/registrar/CMakeFiles/registrar_client.dir/client/registrar_client.cpp.o: modules/registrar/CMakeFiles/registrar_client.dir/flags.make
modules/registrar/CMakeFiles/registrar_client.dir/client/registrar_client.cpp.o: ../modules/registrar/client/registrar_client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object modules/registrar/CMakeFiles/registrar_client.dir/client/registrar_client.cpp.o"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/registrar && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/registrar_client.dir/client/registrar_client.cpp.o -c /home/llogan/labstor/modules/registrar/client/registrar_client.cpp

modules/registrar/CMakeFiles/registrar_client.dir/client/registrar_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/registrar_client.dir/client/registrar_client.cpp.i"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/registrar && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/modules/registrar/client/registrar_client.cpp > CMakeFiles/registrar_client.dir/client/registrar_client.cpp.i

modules/registrar/CMakeFiles/registrar_client.dir/client/registrar_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/registrar_client.dir/client/registrar_client.cpp.s"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/registrar && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/modules/registrar/client/registrar_client.cpp -o CMakeFiles/registrar_client.dir/client/registrar_client.cpp.s

# Object files for target registrar_client
registrar_client_OBJECTS = \
"CMakeFiles/registrar_client.dir/client/registrar_client.cpp.o"

# External object files for target registrar_client
registrar_client_EXTERNAL_OBJECTS =

modules/registrar/libregistrar_client.so: modules/registrar/CMakeFiles/registrar_client.dir/client/registrar_client.cpp.o
modules/registrar/libregistrar_client.so: modules/registrar/CMakeFiles/registrar_client.dir/build.make
modules/registrar/libregistrar_client.so: liblabstor_client_library.so
modules/registrar/libregistrar_client.so: modules/kernel/secure_shmem/libsecure_shmem_client_netlink.so
modules/registrar/libregistrar_client.so: liblabstor_kernel_client.so
modules/registrar/libregistrar_client.so: modules/registrar/CMakeFiles/registrar_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libregistrar_client.so"
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/registrar && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/registrar_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
modules/registrar/CMakeFiles/registrar_client.dir/build: modules/registrar/libregistrar_client.so

.PHONY : modules/registrar/CMakeFiles/registrar_client.dir/build

modules/registrar/CMakeFiles/registrar_client.dir/clean:
	cd /home/llogan/labstor/cmake-build-debug-remote/modules/registrar && $(CMAKE_COMMAND) -P CMakeFiles/registrar_client.dir/cmake_clean.cmake
.PHONY : modules/registrar/CMakeFiles/registrar_client.dir/clean

modules/registrar/CMakeFiles/registrar_client.dir/depend:
	cd /home/llogan/labstor/cmake-build-debug-remote && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/llogan/labstor /home/llogan/labstor/modules/registrar /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote/modules/registrar /home/llogan/labstor/cmake-build-debug-remote/modules/registrar/CMakeFiles/registrar_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/registrar/CMakeFiles/registrar_client.dir/depend

