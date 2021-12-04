#!/bin/bash
NODES=(10.52.0.169 10.52.1.76)
DEVICE=$1
if [ $# -lt 1  ]; then
  echo "USAGE: start_servers [DEVICE: pmem/hdd]"
  exit
fi

for node in ${NODES[@]}
do
echo "Deploying orangefs server on ${node}"
ssh ${node} << EOF
echo "Setting up server at ${node} "
source ~/.bashrc
module load orangefs orangefs-mpich
rm -rf /home/cc/${DEVICE}/data/*
pvfs2-server -f -a ${node} /home/cc/${DEVICE}/orangefs.conf
pvfs2-server -a ${node} /home/cc/${DEVICE}/orangefs.conf
EOF
done
