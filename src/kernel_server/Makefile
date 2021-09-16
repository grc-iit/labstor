obj-m = kernel_server.o
kernel_server-objs += labstor_kernel_server.o
kernel_server-objs += kpkg_devkit/request_queue.o
kernel_server-objs += kpkg_devkit/unordered_map.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean