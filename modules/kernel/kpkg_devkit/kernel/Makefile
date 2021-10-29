EXTRA_CFLAGS=-I$(LABSTOR_INCLUDE) -Wno-missing-braces
obj-m = labstor_kpkg_devkit.o
labstor_kernel_server-objs += request_queue.o
labstor_kernel_server-objs += unordered_map.o
labstor_kernel_server-objs += module_registrar.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean