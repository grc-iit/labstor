#!/bin/bash

sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/src/kernel/server/labstor_kernel_server.ko
sudo insmod ${LABSTOR_ROOT}/labmods/kernel/secure_shmem/kernel/secure_shmem.ko
sudo insmod ${LABSTOR_ROOT}/labmods/kernel/ipc_manager/kernel/ipc_manager.ko
sudo insmod ${LABSTOR_ROOT}/labmods/kernel/work_orchestrator/kernel/work_orchestrator.ko
sudo insmod ${LABSTOR_ROOT}/labmods/kernel/ipc_test/kernel/ipc_test.ko
sudo insmod ${LABSTOR_ROOT}/labmods/kernel/blkdev_table/kernel/blkdev_table.ko
sudo insmod ${LABSTOR_ROOT}/labmods/storage_api/mq_driver/kernel/mq_driver.ko
sudo dmesg
sudo chmod a+rwX /dev/labstor_shared_shmem0