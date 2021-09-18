cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include modules/kernel)

######MAIN REPO
#Build labstor kernel server
add_custom_target(build_labstor_kernel_server ALL COMMAND cd ${CMAKE_SOURCE_DIR}/src/kernel_server && make LABSTOR_INCLUDE=${CMAKE_SOURCE_DIR}/include)
add_custom_target(clean_labstor_kernel_server COMMAND cd ${CMAKE_SOURCE_DIR}/src/kernel_server && make clean)
#Build labstor kernel server client
add_library(labstor_kernel_client ${CMAKE_SOURCE_DIR}/src/kernel_client/kernel_client.cpp)
#Build labstor trusted server
add_executable(labstor_trusted_server src/userspace_server/server.cpp src/userspace_server/worker.cpp src/userspace_server/ipc_manager.cpp)
target_link_libraries(labstor_trusted_server pthread rt labstor_kernel_client)
#Build labstor trusted server client
add_library(labstor_client_library src/userspace_client/client.cpp)

######MODULES
add_subdirectory(modules/kernel/secure_shmem)

#######Unit tests
#Client to trusted server tests
add_executable(client_to_trusted_server test/unit/client_to_trusted_server/test_client.cpp)
add_dependencies(client_to_trusted_server labstor_client_library)
target_link_libraries(client_to_trusted_server labstor_client_library)

#Client to kernel server tests
add_executable(client_to_kernel test/unit/client_to_kernel/test_kernel_client.cpp)
add_dependencies(client_to_kernel labstor_kernel_client)
target_link_libraries(client_to_kernel labstor_kernel_client)

#Shared memory creation
add_executable(test_shmem test/unit/test_shmem/test_shmem.cpp)
add_dependencies(test_shmem labstor_kernel_client secure_shmem_client_netlink)
target_link_libraries(test_shmem labstor_kernel_client secure_shmem_client_netlink)

########UTIL
add_custom_target(start_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server.sh)
add_custom_target(insert_kernel_modules COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/insert_kernel_modules.sh)
add_custom_target(remove_kernel_modules COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/remove_kernel_modules.sh)
add_custom_target(stop_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server.sh)
add_custom_target(start_trusted_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_server.sh)


##########NOTES

#NDCTRL
#ndctl create-namespace --force --reconfig=namespace0.0 --mode=devdax --align=2M

#add_executable(test_ipc_perf test/unit/boost_ipc/read_write_ipc.cpp)
#target_link_libraries(test_ipc_perf rt boost_system mpi)
#add_executable(test_unordered_map test/unit/boost_ipc/unordered_map.cpp)
#target_link_libraries(test_unordered_map rt boost_system pthread mpi)

#LabStor library
#add_library(generic_posix_client src/io_systems/generic_posix/generic_posix_client.cpp)
#add_library(generic_posix_server src/io_systems/generic_posix/generic_posix_server.cpp)