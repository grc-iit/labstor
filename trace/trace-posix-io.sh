#!/bin/bash

#THIS SCRIPT MUST BE RUN IN THE ROOT USER

EXEC=$1
LOG_DIR=$2

sudo sync
sudo sysctl -w vm.drop_caches=3
sudo trace-cmd record -c --max-graph-depth 0 -p function_graph -g __x64_sys_write -F ${EXEC} 1 64k 1 /dev/sdb
trace-cmd report > ${LOG_DIR}/write_record.txt
