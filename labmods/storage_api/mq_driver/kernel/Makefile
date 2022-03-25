LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel/server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/labmods/kernel
STORAGE_APIS=$(CMAKE_SOURCE_DIR)/labmods/storage_api
BLKDEV_TABLE=$(LABSTOR_MODULES)/blkdev_table/kernel

EXTRA_CFLAGS=-I$(LABSTOR_KERNEL_SERVER) -I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -I$(STORAGE_APIS) -DKERNEL_BUILD -Wno-missing-braces# -DDEBUG
KBUILD_EXTRA_SYMBOLS += $(LABSTOR_KERNEL_SERVER)/Module.symvers $(BLKDEV_TABLE)/Module.symvers
obj-m = mq_driver.o
mq_driver-objs += mq_driver_kernel.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean