#!/bin/bash
#NODES=(10.52.2.181 10.52.3.164)
NODES=(10.52.0.169 10.52.1.76)
for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Killing orangefs-client on ${node}"
sudo umount /home/cc/pmem/mount-client
EOF
done
