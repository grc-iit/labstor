#DEVKIT allows module to be built
EXTRA_CFLAGS=-I$(KPKG_DEVKIT)
obj-m = secure_shmem.o
secure_shmem-objs += shmem_kernel.o
secure_shmem-objs += $(KPKG_DEVKIT)/request_queue.o
secure_shmem-objs += $(KPKG_DEVKIT)/unordered_map.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean