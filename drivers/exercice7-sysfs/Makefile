ifneq ($(KERNELRELEASE),)
obj-m += mymodule.o
mymodule-objs := skeleton.o

else
CPU := arm
KDIR := /home/lmi/workspace/xu3/buildroot/output/build/linux-eiafr-5/
TOOLS := /home/lmi/workspace/xu3/buildroot/output/host/usr/bin/arm-linux-
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif


