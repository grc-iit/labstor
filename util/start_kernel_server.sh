#!/bin/bash

LABSTOR_ROOT=/home/lukemartinlogan/Documents/Projects/PhD/labstor/
LABSTOR_BIN=/home/lukemartinlogan/Documents/Projects/PhD/labstor/cmake-build-debug/

NAME=kernel_server

sudo dmesg --clear
sudo insmod ${LABSTOR_ROOT}/src/kernel_server/${NAME}.ko
sudo dmesg | grep labstor_kernel_server