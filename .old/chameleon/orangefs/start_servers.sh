#!/bin/bash
NODES=(10.52.0.169 10.52.1.76)
DEVICE=$1
if [ $# -lt 1  ]; then
  echo "USAGE: start_servers [DEVICE: pmem/hdd]"
  exit
fi

for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Setting up server at ${node} "
rm -rf /home/cc/${DEVICE}/dev_mount/data
rm -rf /home/cc/${DEVICE}/dev_mount/metadata
pvfs2-server -f -a ${node} /home/cc/${DEVICE}/orangefs.conf
pvfs2-server -a ${node} /home/cc/${DEVICE}/orangefs.conf
cat /home/cc/${DEVICE}/server.log
EOF
done
