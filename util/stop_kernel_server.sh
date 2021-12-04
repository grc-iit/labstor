#!/bin/bash

sudo dmesg --clear
sudo rmmod blkdev_table
sudo rmmod work_orchestrator
sudo rmmod ipc_manager
sudo rmmod secure_shmem
sudo rmmod labstor_kernel_server
sudo dmesg