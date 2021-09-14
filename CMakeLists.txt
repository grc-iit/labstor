cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

#LabStor Request Layer KM tests
set(DRIVER_DIR ${CMAKE_SOURCE_DIR}/old/drivers)
set(TRACE_DIR ${CMAKE_SOURCE_DIR}/trace)
add_executable(verify_write ${DRIVER_DIR}/util/verify_write.c)
add_custom_target(build-km COMMAND cd ${DRIVER_DIR}/request_layer/ && make)
add_custom_target(insert-km COMMAND sudo insmod ${DRIVER_DIR}/request_layer/request_layer_km.ko)
add_custom_target(remove-km COMMAND sudo rmmod request_layer_km)
add_custom_target(check-km COMMAND lsmod | grep request_layer_km)
add_custom_target(check-klog COMMAND dmesg | grep request_layer_km)
add_custom_target(clean-klog COMMAND sudo dmesg --clear)
add_custom_target(verify-km COMMAND sudo ${CMAKE_BINARY_DIR}/verify_write /dev/sdb 0 4096 4)
add_custom_target(reset-km COMMAND sudo dd if=/dev/zero of=/dev/sdb bs=4096 count=1)

#LabStor address KM tests
add_executable(kaddr-usr ${CMAKE_SOURCE_DIR}/test/unit/kernel_addr/userspace.cpp)
add_custom_target(build-kaddr-km COMMAND cd ${CMAKE_SOURCE_DIR}/test/unit/kernel_addr && make)
add_custom_target(clean-kaddr-km COMMAND cd ${CMAKE_SOURCE_DIR}/test/unit/kernel_addr && make clean)
add_custom_target(insert-kaddr-km COMMAND sudo insmod ${CMAKE_SOURCE_DIR}/test/unit/kernel_addr/test_addrs.ko)
add_custom_target(remove-kaddr-km COMMAND sudo rmmod test_addrs)
add_custom_target(check-kaddr-km COMMAND lsmod | grep test_addrs)
add_custom_target(check-kaddr-klog COMMAND dmesg | grep test_addrs)
add_custom_target(clean-kaddr-klog COMMAND sudo dmesg --clear)

#Trace POSIX syscalls
add_executable(trace_posix_io_exec ${TRACE_DIR}/trace-posix-io.c)
add_custom_target(trace-posix COMMAND bash ${TRACE_DIR}/trace-posix-io.sh ${CMAKE_BINARY_DIR}/trace_posix_io_exec $ENV{HOME})

#Build the labstor server
add_executable(labstor_trusted_server src/userspace_server/server.cpp src/userspace_server/worker.cpp src/userspace_server/ipc_manager.cpp)
target_link_libraries(labstor_trusted_server pthread rt)

#Build the labstor client
add_library(labstor_client_library src/userspace_client/client.cpp)

#Unit tests
find_package(OpenMP)

#
add_executable(client_to_trusted_server test/unit/client_to_trusted_server/test_client.cpp)
add_dependencies(client_to_trusted_server labstor_client_library)
target_link_libraries(client_to_trusted_server labstor_client_library)

#NDCTRL
#ndctl create-namespace --force --reconfig=namespace0.0 --mode=devdax --align=2M

#add_executable(test_ipc_perf test/unit/boost_ipc/read_write_ipc.cpp)
#target_link_libraries(test_ipc_perf rt boost_system mpi)
#add_executable(test_unordered_map test/unit/boost_ipc/unordered_map.cpp)
#target_link_libraries(test_unordered_map rt boost_system pthread mpi)

#LabStor library
#add_library(generic_posix_client src/io_systems/generic_posix/generic_posix_client.cpp)
#add_library(generic_posix_server src/io_systems/generic_posix/generic_posix_server.cpp)