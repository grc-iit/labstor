
# LabStor

LabStor is a holistic platform for developing and managing I/O stacks in userspace.

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

## 3. Running Experiments

The benchmark directory contains all experiments.

### 3.1. Configuration

Before running any experiments, users must define a few configuration parameters.
An example of such a configuration is "benchmark/conf.yaml", which contains the default
configuration for the tests we ran in Chameleon Cloud. The main parameters of the file are
the paths to devices and mount points to use for experiments. Device paths should be the entire
device (e.g., /dev/sda), not a partion (e.g., /dev/sda1).

```yaml
#benchmark/config.yaml
HDD: /dev/sda
```

If your machine doesn't support all device types (HDD,NVMe,SSD,etc.), you can set the
value to "null" in the conf file. For example,

```yaml
#bencmark/config.yaml
HDD: null
```

### 3.2. Experiment Command

The command to run a test case is structured as follows:
```
export LABSTOR_ROOT=`scspkg pkg-root labstor`
python3 benchmark/test.py [test_case]
```

It will automatically load parameters from config.yaml.

### 3.4. Repeating Blk-Switch Tests

To complete the I/O scheduler experiment, you must switch to the custom kernel compiled
for blk-switch and then install.
```
#Enable kernel 5.4.4
sudo reboot
sudo python3 benchmark/test.py iosched:blkswitch
```

### Individual Tests

If running all tests at once is not required, tests can be executed individually.
To run individual tests, replace [test_case] with the following:
1. io_anatomy
2. live_upgrade
3. work_orch_cpu
4. work_orch_req
5. storage_api
6. iosched_labstor
7. labios
8. filebench
9. iosched_blkswitch