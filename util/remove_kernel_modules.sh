#!/bin/bash

sudo dmesg --clear
sudo rmmod secure_shmem
sudo dmesg | grep labstor_kernel_server