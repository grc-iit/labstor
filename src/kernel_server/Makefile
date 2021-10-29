EXTRA_CFLAGS=-I$(LABSTOR_INCLUDE) -I$(KPKG_DEVKIT_INCLUDE) -Wno-missing-braces
obj-m = labstor_kernel_server.o
labstor_kernel_server-objs += kernel_server.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean