
# LabStor

A high-performance, lightweight, hierarchical, modular storage stack for Linux-based systems.

# 2. Dependencies

Linux 4.15+  
linux-headers-4.15+  
cmake 3.10 or higher  
C++17 compiler  
C11 compiler  

# 3. Building

```
cd /path/to/linux-bio-km   
mkdir build  
cd build
cmake ../
make -j4  
```

```
MODIFY blk_mq_end_request in block/blk-mq.c
ADD RQF_CUSTOM_IO_PATH to include/linux/blkdev.h (1<<24)

IP=192.168.56.1
ssh llogan@$IP -p 4632

rsync -a -e 'ssh -p 4632' --progress /home/lukemartinlogan/Documents/Projects/PhD/labstor llogan@${IP}:~/
rsync -a -e 'ssh -p 4632' --progress /home/lukemartinlogan/Documents/Projects/PhD/custom-ubuntu/linux-stable llogan@${IP}:~/
```
