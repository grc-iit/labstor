LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel_server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/modules/kernel
KPKG_DEVKIT=$(LABSTOR_MODULES)/kpkg_devkit
EXTRA_CFLAGS=-I$(LABSTOR_KERNEL_SERVER) -I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -DKERNEL_BUILD -Wno-missing-braces
KBUILD_EXTRA_SYMBOLS += $(LABSTOR_KERNEL_SERVER)/Module.symvers $(KPKG_DEVKIT)/Module.symvers
obj-m = worker.o
worker-objs += worker_kernel.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean