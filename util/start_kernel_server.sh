#!/bin/bash

NAME=labstor_kernel_server
sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/src/kernel_server/${NAME}.ko
sudo dmesg | grep labstor_kernel_server