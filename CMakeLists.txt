cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/labmods/registrar)

option(DEBUG OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC -DLABSTOR_DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -DLABSTOR_DEBUG")
    message("Debug mode")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC -Ofast")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC -Ofast")
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
        registrar_client
        yaml-cpp)

######UTIL
add_custom_target(start_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server.sh)
add_custom_target(stop_kernel_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server.sh)
add_custom_target(start_trusted_server COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_trusted_server.sh)
add_custom_target(insert_modules COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/install_server_modules.sh)
add_custom_target(start_kernel_server_test COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/start_kernel_server_test.sh)
add_custom_target(stop_kernel_server_test COMMAND LABSTOR_ROOT=${CMAKE_SOURCE_DIR} LABSTOR_BIN=${CMAKE_BINARY_DIR} bash ${CMAKE_SOURCE_DIR}/util/stop_kernel_server_test.sh)
add_custom_target(terminate_server COMMAND ${CMAKE_BUILD_DIR}/labmods/registrar/terminate_server_exec)

add_executable(partitioner util/partitioner.cpp)

add_executable(terminate_server_exec src/util/terminate.cpp)
add_dependencies(terminate_server_exec labstor_client_library registrar_client)
target_link_libraries(terminate_server_exec labstor_client_library registrar_client)

add_executable(modify.labstack src/util/modify.labstack.cpp)
add_dependencies(modify.labstack labstor_client_library registrar_client)
target_link_libraries(modify.labstack labstor_client_library registrar_client yaml-cpp)

add_executable(mount.labstack src/util/mount.labstack.cpp)
add_dependencies(mount.labstack labstor_client_library registrar_client)
target_link_libraries(mount.labstack labstor_client_library registrar_client yaml-cpp)

add_executable(unmount.labstack src/util/unmount.labstack.cpp)
add_dependencies(unmount.labstack labstor_client_library registrar_client)
target_link_libraries(unmount.labstack labstor_client_library registrar_client yaml-cpp)

########INSTALLATION
install(
        TARGETS labstor_kernel_client labstor_server_library labstor_client_library
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(
        TARGETS labstor_trusted_server modify.labstack mount.labstack unmount.labstack partitioner
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})

######MODULES & TESTS
add_subdirectory(labmods)
#add_subdirectory(test)
#add_subdirectory(benchmark)
