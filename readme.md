
# LabStor

LabStor is a holistic platform for developing and managing I/O stacks in userspace.

Update 11/28. We are currently re-organizing the LabStor library to use
in other projects. We are also making the test cases work outside
of our testbed and adding github actions. We are also adding a wiki.
Expected completion: 2 weeks.

[![DOI](https://zenodo.org/badge/499672000.svg)](https://zenodo.org/badge/latestdoi/499672000)

## 1. Installing Dependencies

Our script assumes the following:
* Ubuntu 20.04 (server)
* Linux kernel 5.4.0 (default with ubuntu 20.04)
* gcc/g++ 9.3.0+
* python 3.6+

Otherwise, the installation script will install all dependencies.
```
bash install.sh
source ~/.bashrc
```

These scripts will modify your .bashrc  
When uninstalling, make sure to remove SCSPKG and Jarvis from your bashrc.

### 1.1. Emulating PMEM

If PMEM is not provided on the machine, Linux's PMEM emulator can be configured in GRUB as follows:
```
sudo nano /etc/default/grub
GRUB_CMDLINE_LINUX="memmap=60G!10G" #60G PMEM, starting at offset 10GB in RAM
sudo update-grub2
sudo reboot
```
This is the configuration of PMEM we used in our experiments.

## 2. Building LabStor

```
cd labstor  
mkdir build    
cd build  
module load labstor
cmake ../ -DCMAKE_INSTALL_PREFIX=`scspkg pkg-root labstor`  
make -j4  
make install  
```

### 3. Deploying LabStor

LabStor can be launched as follows:
```
cd labstor
cd build
make start_kernel_server
make start_labstor_server
make stop_kernel_server
make stop_labstor_server
```

To mount a LabStack:
```
mount.labstack [labstack.yaml]
```

## 4. Experiments

The benchmark directory contains some experiments.

NOTE: This currently is only supported for our testbed. We are fixing
this now since LabStor is required in other projects (11/28).

### 4.1. Configuration

Before running any experiments, users must define a few configuration parameters.
An example of such a configuration is "benchmark/conf.yaml", which contains the default
configuration for the tests we ran in Chameleon Cloud. The main parameters of the file are
the paths to devices and mount points to use for experiments. Device paths should be the entire
device (e.g., /dev/sda), not a partion (e.g., /dev/sda1).

```yaml
#benchmark/config.yaml
HDD: /dev/sda
```

### 4.2. Experiment Command

The command to run a test case is structured as follows:
```
export LABSTOR_ROOT=`scspkg pkg-root labstor`
python3 benchmark/test.py [test_case]
```

Test cases:
1. io_anatomy
2. live_upgrade
3. work_orch_cpu
4. work_orch_req
5. storage_api
6. iosched_labstor
7. filebench
8. iosched_blkswitch

It will automatically load parameters from config.yaml.

### 4.3. Blk-Switch Tests

To complete the I/O scheduler experiment, you must switch to the custom kernel compiled
for blk-switch and then install.
```
#Enable kernel 5.4.4
sudo reboot
sudo python3 benchmark/test.py iosched:blkswitch
```
