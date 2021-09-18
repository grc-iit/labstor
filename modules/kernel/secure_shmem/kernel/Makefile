#DEVKIT allows module to be built
EXTRA_CFLAGS=-I$(KERNEL_SERVER) -I$(SOURCE_DIR) -I$(LABSTOR_INCLUDE) -Wno-missing-braces
KBUILD_EXTRA_SYMBOLS += $(KERNEL_SERVER)/Module.symvers
obj-m = secure_shmem.o
secure_shmem-objs += secure_shmem_kernel.o
#secure_shmem-objs += $(KERNEL_SERVER)/kernel_server.o
#secure_shmem-objs += $(KERNEL_SERVER)/kpgk_devkit/request_queue.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean