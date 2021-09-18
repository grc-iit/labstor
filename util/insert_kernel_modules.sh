#!/bin/bash

sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/modules/kernel/secure_shmem/kernel/shmem_kernel.ko
sudo dmesg | grep labstor_kernel_server