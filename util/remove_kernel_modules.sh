#!/bin/bash

sudo dmesg --clear
sudo rmmod shmem_kernel
sudo dmesg | grep labstor_kernel_server