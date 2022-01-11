#!/bin/bash

sudo dmesg --clear
sudo rmmod bio_driver
sudo rmmod mq_driver
sudo rmmod blkdev_table
sudo rmmod ipc_test
sudo rmmod work_orchestrator
sudo rmmod ipc_manager
sudo rmmod secure_shmem
sudo rmmod labstor_kernel_server
sudo dmesg