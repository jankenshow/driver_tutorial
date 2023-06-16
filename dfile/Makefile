CFILES = dfile.c

obj-m := DFile.o
DFile-objs := $(CFILES:.c=.o)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

load:
	sudo insmod DFile.ko && sudo mknod /dev/dFile c 63 1 && sudo chmod 666 /dev/dFile

unload:
	sudo rmmod DFile && sudo rm /dev/dFile

write:
	echo "a" > /dev/dFile

read:
	cat /dev/dFile

check:
	cat /proc/devices | grep DFile && ls -la /dev | grep dFile

check_messagemessage:
	sudo dmesg | grep dfile
