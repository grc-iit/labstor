#!/bin/bash

sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/src/kernel/server/labstor_kernel_server.ko
sudo insmod ${LABSTOR_ROOT}/modules/kernel/secure_shmem/kernel/secure_shmem.ko
sudo insmod ${LABSTOR_ROOT}/test/unit/request_queue_client_kernel/request_queue_km.ko
sudo dmesg
sudo chmod a+rwX /dev/labstor_shared_shmem0