LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel/server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/labmods/kernel
LABSTOR_SECURE_SHMEM_MODULE=$(LABSTOR_MODULES)/secure_shmem/kernel

EXTRA_CFLAGS=-I$(LABSTOR_KERNEL_SERVER) -I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -DKERNEL_BUILD -Wno-missing-braces
KBUILD_EXTRA_SYMBOLS += $(LABSTOR_KERNEL_SERVER)/Module.symvers  $(LABSTOR_SECURE_SHMEM_MODULE)/Module.symvers
obj-m = ipc_manager.o
ipc_manager-objs += ipc_manager_kernel.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean