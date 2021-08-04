#!/bin/bash
DEVPATH=$1
DEVICE=$2
FORMAT=$3
NODES=(10.52.0.169 10.52.1.76)

if [ $# -lt 3  ]; then
  echo "USAGE: start_clients [DEVPATH: /dev/pmem0] [DEVICE: pmem/hdd] [FORMAT: ext4-dax/ext4/xfs]"
  exit
fi

if [ ${FORMAT} == "ext4-dax" ]; then

for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Mounting EXT4 dax on ${node}"
sudo umount /home/cc/${DEVICE}/dev_mount
sudo mkfs.ext4 -F ${DEVPATH}
sudo mount -o dax ${DEVPATH} /home/cc/${DEVICE}/dev_mount
sudo chown -R cc /home/cc/${DEVICE}/dev_mount
EOF
done

elif [ ${FORMAT} == "ext4" ]; then

for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Mounting EXT4 on ${node} /home/cc/${DEVICE}/dev_mount"
sudo umount /home/cc/${DEVICE}/dev_mount
sudo mkfs.ext4 -F ${DEVPATH}
sudo mount ${DEVPATH} /home/cc/${DEVICE}/dev_mount
sudo chown -R cc /home/cc/${DEVICE}/dev_mount
EOF
done

elif [ ${FORMAT} == "xfs" ]; then

for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Mounting XFS on ${node}"
mkdir -p /home/cc/${DEVICE}/dev_mount
sudo umount /home/cc/${DEVICE}/dev_mount
sudo mkfs.xfs -f ${DEVPATH}
sudo mount ${DEVPATH} /home/cc/${DEVICE}/dev_mount
sudo chown -R cc /home/cc/${DEVICE}/dev_mount
EOF
done

elif [ ${FORMAT} == "xfs-dax" ]; then

for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "Mounting XFS on ${node}"
mkdir -p /home/cc/${DEVICE}/dev_mount
sudo umount /home/cc/${DEVICE}/dev_mount
sudo mkfs.xfs -f ${DEVPATH}
sudo mount -o dax ${DEVPATH} /home/cc/${DEVICE}/dev_mount
sudo chown -R cc /home/cc/${DEVICE}/dev_mount
EOF
done

fi
