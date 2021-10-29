LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel_server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/modules
KPKG_DEVKIT=$(LABSTOR_MODULES)/kpkg_devkit
SECURE_SHMEM=$(LABSTOR_MODULES)/secure_shmem
EXTRA_CFLAGS=-I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -DKERNEL_BUILD -Wno-missing-braces
obj-m = labstor_kpkg_devkit.o
labstor_kpkg_devkit-objs += request_queue.o
labstor_kpkg_devkit-objs += unordered_map.o
labstor_kpkg_devkit-objs += module_registrar.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean