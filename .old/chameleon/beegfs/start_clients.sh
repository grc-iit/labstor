#!/bin/bash
NODES=(10.52.2.181 10.52.3.164)
MAIN_SERVER=10.52.0.169
DEVICE=$1
if [ $# -lt 1  ]; then
  echo "USAGE: start_clients [DEVICE: pmem/hdd]"
  exit
fi

for node in ${NODES[@]}
do
echo "Deploying orangefs client on ${node}"
ssh ${node} << EOF
echo "Starting client on ${node}"
source ~/.bashrc
module load orangefs orangefs-mpich
export PVFS2TAB_FILE=/home/cc/${DEVICE}/pvfs2tab
export MOUNT_PATH=/home/cc/${DEVICE}/mount-client
mkdir -p ${MOUNT_PATH}
echo "tcp://${MAIN_SERVER}:3334/orangefs ${MOUNT_PATH} pvfs2 defaults,noauto 0 0" >> ${PVFS2TAB_FILE}
pvfs2fuse ${MOUNT_PATH} -o fs_spec=tcp://${MAIN_SERVER}:3334/orangefs
EOF
done
