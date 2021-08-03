cmake_minimum_required(VERSION 3.10)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

add_executable(verify_write ${CMAKE_SOURCE_DIR}/drivers/util/verify_write.c)

add_custom_target(build-km COMMAND cd ${CMAKE_SOURCE_DIR}/drivers/request_layer/ && make)

add_custom_target(insert-km COMMAND sudo insmod ${CMAKE_SOURCE_DIR}/drivers/request_layer/request_layer_km.ko)
add_custom_target(remove-km COMMAND sudo rmmod request_layer_km)
add_custom_target(check-km COMMAND lsmod | grep request_layer_km)
add_custom_target(check-klog COMMAND dmesg | grep request_layer_km)
add_custom_target(clean-klog COMMAND sudo dmesg --clear)
add_custom_target(verify-km COMMAND sudo ${CMAKE_BINARY_DIR}/verify_write /dev/sdb 0 4096 4)

add_executable(trace_posix_io_exec ${CMAKE_SOURCE_DIR}/trace/trace-posix-io.c)
add_custom_target(trace-posix COMMAND bash ${CMAKE_SOURCE_DIR}/trace/trace-posix-io.sh ${CMAKE_BINARY_DIR}/trace_posix_io_exec $ENV{HOME})