LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel/server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/labmods/kernel
LABSTOR_SHMEM_MODULE=$(LABSTOR_MODULES)/secure_shmem/kernel
LABSTOR_IPC_MANAGER_MODULE=$(LABSTOR_MODULES)/ipc_manager/kernel

EXTRA_CFLAGS=-I$(LABSTOR_KERNEL_SERVER) -I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -DKERNEL_BUILD -Wno-missing-braces
KBUILD_EXTRA_SYMBOLS += $(LABSTOR_KERNEL_SERVER)/Module.symvers $(LABSTOR_SHMEM_MODULE)/Module.symvers $(LABSTOR_IPC_MANAGER_MODULE)/Module.symvers
obj-m = work_orchestrator.o
work_orchestrator-objs += work_orchestrator_kernel.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean