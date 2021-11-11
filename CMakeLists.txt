cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include modules/kernel)

option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)


######KERNEL SERVER
set(KPKG_DEVKIT_INCLUDE )
add_custom_target(build_labstor_kernel_server ALL COMMAND
        cd ${CMAKE_SOURCE_DIR}/src/kernel_server && make
        CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(build_labstor_kernel_server build_kpkg_devkit)
add_custom_target(clean_labstor_kernel_server COMMAND cd ${CMAKE_SOURCE_DIR}/src/kernel_server && make clean)

######KERNEL CLIENT
add_library(labstor_kernel_client ${CMAKE_SOURCE_DIR}/src/kernel_client/kernel_client.cpp)

######LABSTOR USERSPACE SERVER
add_executable(labstor_trusted_server
        src/userspace_server/server.cpp
        src/userspace_server/worker.cpp
        src/userspace_server/ipc_manager.cpp
        src/userspace_server/work_orchestrator.cpp
        src/userspace_server/module_manager.cpp)
add_dependencies(labstor_trusted_server
        labstor_kernel_client
        secure_shmem_client_netlink
        worker_client_netlink)
target_link_libraries(labstor_trusted_server
        pthread rt dl
        labstor_kernel_client
        yaml-cpp
        secure_shmem_client_netlink
        worker_client_netlink)

######LABSTOR USERSPACE CLIENT
add_library(labstor_client_library
        src/userspace_client/client.cpp
        src/userspace_client/ipc_manager.cpp
        src/userspace_client/module_manager.cpp)
add_dependencies(labstor_client_library
        labstor_kernel_client
        secure_shmem_client_netlink)
target_link_libraries(labstor_client_library
        pthread rt dl
        labstor_kernel_client
        secure_shmem_client_netlink)

######UTIL
add_custom_target(start_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server.sh)
add_custom_target(stop_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server.sh)
add_custom_target(start_trusted_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_server.sh)
add_custom_target(start_kernel_server_test COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server_test.sh)
add_custom_target(stop_kernel_server_test COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server_test.sh)

######MODULES & TESTS
add_subdirectory(modules)
add_subdirectory(test)
