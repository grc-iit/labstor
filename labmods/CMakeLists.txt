cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

option(WITH_SPDK "Include SPDK tests" OFF)

include_directories(include kernel filesystems iosched)

##MODULES WITH KERNEL COMPONENT
add_subdirectory(kernel/secure_shmem)
add_subdirectory(kernel/ipc_manager)
add_subdirectory(kernel/work_orchestrator)
add_subdirectory(kernel/blkdev_table)
add_subdirectory(storage_api/mq_driver)
add_subdirectory(kernel/ipc_test)
add_custom_target(clean_modules COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean &&
        cd ${CMAKE_BINARY_DIR} && make clean_labstor_kernel_server &&
        cd ${CMAKE_BINARY_DIR} && make clean_secure_shmem &&
        cd ${CMAKE_BINARY_DIR} && make clean_ipc_manager &&
        cd ${CMAKE_BINARY_DIR} && make clean_ipc_test &&
        cd ${CMAKE_BINARY_DIR} && make clean_blkdev_table &&
        cd ${CMAKE_BINARY_DIR} && make clean_mq_driver &&
        cd ${CMAKE_BINARY_DIR} && make clean_work_orchestrator)

######MODULES
add_subdirectory(registrar)
add_subdirectory(dummy)
add_subdirectory(filesystems/generic_posix)
add_subdirectory(filesystems/labstor_fs)

message("Are we with SPDK?")
message(${WITH_SPDK})
if(${WITH_SPDK})
add_subdirectory(storage_api/spdk)
endif()