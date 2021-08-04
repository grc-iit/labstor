#!/bin/bash
#NODES=(10.52.2.181 10.52.3.164)
NODES=(10.52.0.169 10.52.1.76)
MAIN_SERVER=10.52.0.169
DEVICE=$1
MOUNT_PATH=/home/cc/${DEVICE}/mount-client
PVFS2TAB_FILE=/home/cc/${DEVICE}/pvfs2tab
if [ $# -lt 1  ]; then
  echo "USAGE: start_clients [DEVICE: pmem/hdd]"
  exit
fi

for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Starting client on ${node}"
rm -rf ${MOUNT_PATH}
mkdir -p ${MOUNT_PATH}
echo "tcp://${MAIN_SERVER}:3334/orangefs /home/cc/${DEVICE}/mount-client pvfs2 defaults,noauto 0 0" > ${PVFS2TAB_FILE}
pvfs2fuse ${MOUNT_PATH} -o fs_spec=tcp://${MAIN_SERVER}:3334/orangefs
EOF
done
