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
include CMakeFiles/labstor_client_library.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/labstor_client_library.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/labstor_client_library.dir/flags.make

CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.o: CMakeFiles/labstor_client_library.dir/flags.make
CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.o: ../src/userspace/client/singleton.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.o -c /home/llogan/labstor/src/userspace/client/singleton.cpp

CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/src/userspace/client/singleton.cpp > CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.i

CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/src/userspace/client/singleton.cpp -o CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.s

CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.o: CMakeFiles/labstor_client_library.dir/flags.make
CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.o: ../src/userspace/client/client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.o -c /home/llogan/labstor/src/userspace/client/client.cpp

CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/src/userspace/client/client.cpp > CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.i

CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/src/userspace/client/client.cpp -o CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.s

CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.o: CMakeFiles/labstor_client_library.dir/flags.make
CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.o: ../src/userspace/client/ipc_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.o -c /home/llogan/labstor/src/userspace/client/ipc_manager.cpp

CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/src/userspace/client/ipc_manager.cpp > CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.i

CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/src/userspace/client/ipc_manager.cpp -o CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.s

CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.o: CMakeFiles/labstor_client_library.dir/flags.make
CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.o: ../src/userspace/client/module_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.o -c /home/llogan/labstor/src/userspace/client/module_manager.cpp

CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/src/userspace/client/module_manager.cpp > CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.i

CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/src/userspace/client/module_manager.cpp -o CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.s

CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.o: CMakeFiles/labstor_client_library.dir/flags.make
CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.o: ../modules/registrar/client/registrar_client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.o -c /home/llogan/labstor/modules/registrar/client/registrar_client.cpp

CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llogan/labstor/modules/registrar/client/registrar_client.cpp > CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.i

CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llogan/labstor/modules/registrar/client/registrar_client.cpp -o CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.s

# Object files for target labstor_client_library
labstor_client_library_OBJECTS = \
"CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.o" \
"CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.o" \
"CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.o" \
"CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.o" \
"CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.o"

# External object files for target labstor_client_library
labstor_client_library_EXTERNAL_OBJECTS =

liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/src/userspace/client/singleton.cpp.o
liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/src/userspace/client/client.cpp.o
liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/src/userspace/client/ipc_manager.cpp.o
liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/src/userspace/client/module_manager.cpp.o
liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/modules/registrar/client/registrar_client.cpp.o
liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/build.make
liblabstor_client_library.so: modules/kernel/secure_shmem/libsecure_shmem_client_netlink.so
liblabstor_client_library.so: liblabstor_kernel_client.so
liblabstor_client_library.so: CMakeFiles/labstor_client_library.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/llogan/labstor/cmake-build-debug-remote/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX shared library liblabstor_client_library.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/labstor_client_library.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/labstor_client_library.dir/build: liblabstor_client_library.so

.PHONY : CMakeFiles/labstor_client_library.dir/build

CMakeFiles/labstor_client_library.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/labstor_client_library.dir/cmake_clean.cmake
.PHONY : CMakeFiles/labstor_client_library.dir/clean

CMakeFiles/labstor_client_library.dir/depend:
	cd /home/llogan/labstor/cmake-build-debug-remote && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/llogan/labstor /home/llogan/labstor /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote /home/llogan/labstor/cmake-build-debug-remote/CMakeFiles/labstor_client_library.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/labstor_client_library.dir/depend

