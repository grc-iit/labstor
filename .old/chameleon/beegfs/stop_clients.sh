#!/bin/bash
NODES=(10.52.2.181 10.52.3.164)
for node in ${NODES[@]}
do
echo "Killing orangefs client on ${node}"
ssh ${node} << EOF
export MOUNT_PATH=/home/cc/pmem/mount-client
sudo umount ${MOUNT_PATH}
EOF
done
