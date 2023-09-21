obj-m += module_programming.o

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
    insmod module_programming.ko

unload:
    rmmod module_programming.ko

show_buffer:
    dmesg | tail
