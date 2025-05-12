obj-m += asus_tuf_m3_mouse.o
ccflags-y += -msse -mpreferred-stack-boundary=4

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
