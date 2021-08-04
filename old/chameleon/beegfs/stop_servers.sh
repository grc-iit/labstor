NODES=(10.52.0.169 10.52.1.76)
for node in ${NODES[@]}
do
ssh ${node} << EOF
echo "killing pvfs2 on ${node}"
source ~/.bashrc
module load orangefs orangefs-mpich
pkill pvfs2-server
EOF
done
