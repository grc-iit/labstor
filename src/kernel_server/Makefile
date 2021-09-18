EXTRA_CFLAGS=-I$(LABSTOR_INCLUDE) -Wno-missing-braces
obj-m = labstor_kernel_server.o
labstor_kernel_server-objs += kernel_server.o
labstor_kernel_server-objs += kpkg_devkit/request_queue.o
labstor_kernel_server-objs += kpkg_devkit/unordered_map.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean