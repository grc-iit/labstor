#!/bin/bash

LABSTOR_ROOT=/home/lukemartinlogan/Documents/Projects/PhD/labstor/
LABSTOR_BIN=/home/lukemartinlogan/Documents/Projects/PhD/labstor/cmake-build-debug/

sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/modules/kernel/secure_shmem/kernel/shmem_kernel.ko
sudo dmesg | grep labstor_kernel_server