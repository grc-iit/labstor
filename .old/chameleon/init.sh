#!/bin/bash
NODES=(10.52.0.169 10.52.1.76 10.52.2.181 10.52.3.164)
DEVICES=(pmem hdd)

for node in ${NODES[@]}
do
echo "Deploying orangefs client on ${node}"
ssh ${node} << EOF
mkdir -p /home/cc/pmem/dev_mount/data
mkdir -p /home/cc/pmem/dev_mount/metadata
mkdir -p /home/cc/hdd/dev_mount/data
mkdir -p /home/cc/hdd/dev_mount/metadata
EOF
done
