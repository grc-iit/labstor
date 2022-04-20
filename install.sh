#!/bin/bash

#Create the LabStor lib directory
mkidr deps
cd deps

########DETECT DISTRO
if command -v apt &> /dev/null
then
  IS_DEBIAN=true
elif command -v yum &> /dev/null
then
  IS_RED_HAT=true
else
  echo "Error: neither apt or yum is installed"
  exit
fi

########FROM DISTRIBUTION

#Kernel headers
if $IS_DEBIAN
then
  sudo apt install -y linux-headers-`uname -r` libaio-dev
elif $IS_RED_HAT
then
  sudo yum -y install linux-headers-`uname -r`
fi

########FROM SOURCE

#Install SCSPKG
git clone https://github.com/lukemartinlogan/scspkg.git
cd scspkg
bash install.sh
source ~/.bashrc

#Install Jarvis-CD
git clone https://github.com/lukemartinlogan/jarvis-cd.git
cd jarvis-cd
bash install.sh
source ~/.bashrc

#Install CMAKE


#Install MPICH
scspkg create mpich
cd `scspkg pkg-src mpich`
wget http://www.mpich.org/static/downloads/3.2/mpich-3.2.tar.gz --no-check-certificate
tar -xzf mpich-3.2.tar.gz
cd mpich-3.2
./configure --prefix=`scspkg pkg-root mpich` --enable-fast=O3 --enable-romio --enable-shared
make -j8
make install

#Install Yaml-CPP
scspkg create yaml-cpp
cd `scspkg pkg-src yaml-cpp`
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
git checkout db6deedcd301754723065e0bbb1b75927c5b49c7
mkdir build
cd build
cmake ../  -DYAML_BUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=`scspkg pkg-root yaml-cpp`
make -j8
make install

#FIO
scspkg create fio
cd `scspkg pkg-src fio`
wget https://github.com/axboe/fio/archive/refs/tags/fio-3.28.tar.gz
tar -xzf fio-3.28.tar.gz
cd fio
./configure --prefix=`scspkg pkg-root fio`
make -j8
make install

#SPDK
scspkg create spdk
cd `scspkg pkg-src spdk`
git clone https://github.com/spdk/spdk
cd spdk
git checkout v21.10
git submodule update --init
sudo scripts/pkgdep.sh --all
./configure --prefix=`scspkg pkg-root spdk`
make -j8
make install
export SPDK_ROOT=`scspkg pkg-src spdk`/spdk

#LIBURING
scspkg create liburing
cd `scspkg pkg-src liburing`
git clone https://github.com/axboe/liburing.git
cd liburing
./configure --prefix=`scspkg pkg-root liburing`
make -j8
make install

#FxMark
#https://www.usenix.org/system/files/conference/atc16/atc16_paper-min.pdf
git clone https://github.com/sslab-gatech/fxmark.git
cd fxmark
make
#sudo bin/fxmark --type=MWCL --ncore=1 --duration=20 --root=/home/cc/hi

#Filebench
scspkg create filebench
cd `scspkg pkg-src filebench`
wget https://github.com/filebench/filebench/archive/refs/tags/1.4.9.1.tar.gz
tar -xzf 1.4.9.1.tar.gz
cd filebench
libtoolize
aclocal
autoheader
automake --add-missing
autoconf
./configure --prefix=`scspkg pkg-root filebench`
make -j8
make install