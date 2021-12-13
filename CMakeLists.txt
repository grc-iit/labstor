cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/modules/registrar)

option(DEBUG OFF)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(CMAKE_C_FLAGS "-g -fPIC -DLABSTOR_DEBUG")
    set(CMAKE_CXX_FLAGS "-g -fPIC -DLABSTOR_DEBUG")
    message("Debug mode")
else()
    set(CMAKE_C_FLAGS "-g -fPIC -Ofast")
    set(CMAKE_CXX_FLAGS "-g -fPIC -Ofast")
    message("Optimized mode")
endif()
option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)

######CUSTOM DATA STRUCTURES
add_custom_target(build_data_structures COMMAND
        python3 ${CMAKE_SOURCE_DIR}/preprocess.py ${CMAKE_SOURCE_DIR})

######KERNEL SERVER
add_custom_target(build_labstor_kernel_server ALL COMMAND
        cd ${CMAKE_SOURCE_DIR}/src/kernel/server && make
        CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(clean_labstor_kernel_server COMMAND cd ${CMAKE_SOURCE_DIR}/src/kernel/server && make clean)

######KERNEL CLIENT
add_library(labstor_kernel_client
        src/kernel/client/singleton.cpp
        src/kernel/client/kernel_client.cpp)

######LABSTOR USERSPACE SERVER LIB
add_library(labstor_server_library
        src/userspace/server/singleton.cpp
        src/userspace/server/worker.cpp
        src/userspace/server/module_manager.cpp
        src/userspace/server/ipc_manager.cpp
        src/userspace/server/work_orchestrator.cpp
        src/userspace/server/namespace.cpp)
add_dependencies(labstor_server_library
        labstor_kernel_client
        secure_shmem_client_netlink
        ipc_manager_client_netlink
        work_orchestrator_client_netlink
        registrar_server)
target_link_libraries(labstor_server_library
        pthread rt dl
        labstor_kernel_client
        yaml-cpp
        secure_shmem_client_netlink
        ipc_manager_client_netlink
        work_orchestrator_client_netlink
        registrar_server)

######LABSTOR USERSPACE SERVER
add_executable(labstor_trusted_server src/userspace/server/server.cpp)
add_dependencies(labstor_trusted_server labstor_server_library)
target_link_libraries(labstor_trusted_server labstor_server_library)

######LABSTOR USERSPACE CLIENT
add_library(labstor_client_library
        src/userspace/client/singleton.cpp
        src/userspace/client/client.cpp
        src/userspace/client/ipc_manager.cpp
        src/userspace/client/module_manager.cpp)
add_dependencies(labstor_client_library
        labstor_kernel_client
        secure_shmem_client_netlink
        registrar_client)
target_link_libraries(labstor_client_library
        pthread rt dl
        labstor_kernel_client
        secure_shmem_client_netlink
        registrar_client)

######UTIL
add_executable(proc_partition util/partitioner.cpp)
add_custom_target(start_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server.sh)
add_custom_target(stop_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server.sh)
add_custom_target(start_trusted_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_trusted_server.sh)
add_custom_target(start_kernel_server_test COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server_test.sh)
add_custom_target(stop_kernel_server_test COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server_test.sh)

######MODULES & TESTS
add_subdirectory(modules)
add_subdirectory(test)
