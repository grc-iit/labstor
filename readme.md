
# LabStor

A high-performance, lightweight, hierarchical, modular storage stack for Linux-based systems.

# 2. Dependencies

Linux 5.4.0-77-generic  
linux-headers-5.4+  
cmake 3.10 or higher  
C++17 compiler  
C11 compiler  
yaml-cpp

```
scspkg create yaml-cpp
cd `scspkg pkg-src yaml-cpp` 
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
git checkout db6deedcd301754723065e0bbb1b75927c5b49c7
mkdir build
cd build
cmake ../  -DYAML_BUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=`scspkg pkg-root yaml-cpp`
```

# 3. Building

```
cd /path/to/labstor
mkdir build  
cd build
cmake ../
make -j4  
```

```
IP=192.168.56.1
ssh llogan@$IP -p 4632

rsync -a -e 'ssh -p 4632' --progress /home/lukemartinlogan/Documents/Projects/PhD/labstor llogan@${IP}:~/
rsync -a -e 'ssh -p 4632' --progress /home/lukemartinlogan/Documents/Projects/PhD/custom-ubuntu/linux-stable llogan@${IP}:~/
```
