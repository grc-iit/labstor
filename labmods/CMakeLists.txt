cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include kernel filesystems iosched)

##MODULES WITH KERNEL COMPONENT
add_subdirectory(secure_shmem)
add_subdirectory(ipc_manager)
add_subdirectory(work_orchestrator)
add_subdirectory(blkdev_table)
add_subdirectory(mq_driver)
add_subdirectory(ipc_test)
add_custom_target(clean_modules COMMAND
        cd ${CMAKE_BINARY_DIR} && make clean &&
        cd ${CMAKE_BINARY_DIR} && make clean_labstor_kernel_server &&
        cd ${CMAKE_BINARY_DIR} && make clean_secure_shmem &&
        cd ${CMAKE_BINARY_DIR} && make clean_ipc_manager &&
        cd ${CMAKE_BINARY_DIR} && make clean_ipc_test &&
        cd ${CMAKE_BINARY_DIR} && make clean_blkdev_table &&
        cd ${CMAKE_BINARY_DIR} && make clean_mq_driver &&
        cd ${CMAKE_BINARY_DIR} && make clean_work_orchestrator)

######FULLY USERLAND MODULES
add_subdirectory(block_fs)
add_subdirectory(dummy)
add_subdirectory(generic_block)
add_subdirectory(generic_posix)
add_subdirectory(generic_queue)
#add_subdirectory(labstor_fs)
#add_subdirectory(LRU)
add_subdirectory(no_op)
add_subdirectory(registrar)
#add_subdirectory(time_keeper)

if(${SPDK_BUILD_DIR})
    messsage("Building SPDK")
    add_subdirectory(spdk)
endif()