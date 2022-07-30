#!/bin/bash

mkdir external

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
  sudo apt install -y linux-headers-`uname -r` libaio-dev maven
elif $IS_RED_HAT
then
  sudo yum -y install linux-headers-`uname -r`
fi

########FROM SOURCE

LABSTOR_ROOT=`pwd`

#LABSTOR
scspkg create labstor

#Install Jarvis-CD
echo "INSTALLING JARVIS"
cd external
git clone https://github.com/lukemartinlogan/jarvis-cd.git -b vLabstor
cd jarvis-cd
bash dependencies.sh
source ~/.bashrc
python3 -m pip install -e . --user -r requirements.txt
jarvis deps scaffold labstor
jarvis deps local-install all
source ~/.bashrc
cd ${LABSTOR_ROOT}
echo
echo

#Install SCSPKG
echo INSTALLING SCSPKG
cd external
git clone https://github.com/scs-lab/scspkg.git
cd scspkg
git switch v0.1.0
bash install.sh
source ~/.bashrc
cd ${LABSTOR_ROOT}
echo
echo

#Install CMAKE
echo INSTALLING CMAKE
spack install cmake@3.22.1
spack load cmake
scspkg from-spack cmake@3.22.1 cmake-labstor
echo
echo

#Install Yaml-CPP
echo INSTALLING YAML-CPP
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
echo
echo

#FIO
echo INSTALLING FIO
scspkg create fio
cd `scspkg pkg-src fio`
wget https://github.com/axboe/fio/archive/refs/tags/fio-3.28.tar.gz
tar -xzf fio-3.28.tar.gz
cd fio-fio-3.28
./configure --prefix=`scspkg pkg-root fio`
make -j8
make install
echo
echo

#SPDK
echo INSTALLING SPDK
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
echo
echo

#LIBURING
echo INSTALLING LIBURING
scspkg create liburing
cd `scspkg pkg-src liburing`
git clone https://github.com/axboe/liburing.git
cd liburing
./configure --prefix=`scspkg pkg-root liburing`
make -j8
make install
echo
echo

#FxMark
echo INSTALLING FXMARK
#https://www.usenix.org/system/files/conference/atc16/atc16_paper-min.pdf
scspkg create fxmark
cd `scspkg pkg-src fxmark`
git clone https://github.com/sslab-gatech/fxmark.git
cd fxmark
make
cp -r bin `scspkg pkg-root fxmark`/bin
echo
echo

#Filebench
scspkg create filebench
cd `scspkg pkg-src filebench`
wget https://github.com/filebench/filebench/archive/refs/tags/1.4.9.1.tar.gz
tar -xzf 1.4.9.1.tar.gz
cd filebench-1.4.9.1
libtoolize
aclocal
autoheader
automake --add-missing
autoconf
./configure --prefix=`scspkg pkg-root filebench`
make -j8
make install

#Install MPICH
scspkg create mpich
cd `scspkg pkg-src mpich`
wget http://www.mpich.org/static/downloads/3.2/mpich-3.2.tar.gz --no-check-certificate
tar -xzf mpich-3.2.tar.gz
cd mpich-3.2
./configure --prefix=`scspkg pkg-root mpich` --enable-fast=O3 --enable-romio --enable-shared
make -j8
make install

#Add LabStor package dependencies
scspkg add-deps labstor filebench fxmark liburing spdk fio yaml-cpp cmake-labstor mpich
