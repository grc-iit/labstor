LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel_server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/modules/kernel
KPKG_DEVKIT=$(LABSTOR_MODULES)/kpkg_devkit
SECURE_SHMEM=$(LABSTOR_MODULES)/secure_shmem/kernel
EXTRA_CFLAGS=-I$(LABSTOR_KERNEL_SERVER) -I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -DKERNEL_BUILD -Wno-missing-braces
KBUILD_EXTRA_SYMBOLS += $(LABSTOR_KERNEL_SERVER)/Module.symvers $(KPKG_DEVKIT)/Module.symvers $(SECURE_SHMEM)/Module.symvers
obj-m = request_queue_km.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean