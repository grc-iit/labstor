LABSTOR_KERNEL_SERVER=$(CMAKE_SOURCE_DIR)/src/kernel/server
LABSTOR_INCLUDE=$(CMAKE_SOURCE_DIR)/include
LABSTOR_MODULES=$(CMAKE_SOURCE_DIR)/labmods/kernel

EXTRA_CFLAGS=-I$(LABSTOR_INCLUDE) -I$(LABSTOR_MODULES) -DKERNEL_BUILD -Wno-missing-braces
obj-m = labstor_kernel_server.o
labstor_kernel_server-objs += kernel_server.o
labstor_kernel_server-objs += module_manager.o
KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean