#!/bin/bash

sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/modules/kernel/secure_shmem/kernel/secure_shmem.ko
sudo dmesg | grep labstor_kernel_server