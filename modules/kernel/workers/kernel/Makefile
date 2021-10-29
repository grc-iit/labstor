#DEVKIT allows module to be built
EXTRA_CFLAGS=-I$(KERNEL_SERVER) -I$(SOURCE_DIR) -I$(LABSTOR_INCLUDE) -I$(KPKG_DEVKIT_INCLUDE) -Wno-missing-braces
KBUILD_EXTRA_SYMBOLS += $(KERNEL_SERVER)/Module.symvers
obj-m = worker.o
worker-objs += worker_kernel.o
#worker-objs += $(KERNEL_SERVER)/kernel_server.o
#worker-objs += $(KERNEL_SERVER)/kpgk_devkit/request_queue.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean