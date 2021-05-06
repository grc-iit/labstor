cmake_minimum_required(VERSION 3.19)
project(labstor)

set(CMAKE_CXX_STANDARD 17)

add_custom_target(build-km COMMAND bash ${BUILD_LINUX_DRIVER_IO_SH} 1 ${TIME_LINUX_DRIVER_IO_ROOT})
add_custom_target(insert-km COMMAND sudo insmod ${TIME_LINUX_DRIVER_IO_EXEC})
add_custom_target(remove-km COMMAND sudo rmmod time_linux_driver_io_km)
add_custom_target(check-km COMMAND lsmod | grep time_linux_driver_io_km)
add_custom_target(check-klog COMMAND dmesg | grep time_linux_driver_io_km)
add_custom_target(clean-km COMMAND bash ${BUILD_LINUX_DRIVER_IO_SH} 2 ${TIME_LINUX_DRIVER_IO_ROOT})
add_custom_target(clean-klog COMMAND sudo dmesg --clear)