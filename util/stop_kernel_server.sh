#!/bin/bash

sudo dmesg --clear
sudo rmmod secure_shmem
sudo rmmod labstor_kernel_server
sudo dmesg