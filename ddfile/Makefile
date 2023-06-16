CFILES = ddfile.c

obj-m := DDFile.o
DDFile-objs := $(CFILES:.c=.o)

ccflags-y += -std=gnu99 -Wall -Wno-declaration-after-statement

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

# add_rule:
# 	sudo -i 
# 	echo 'KERNEL=="ddfile[0-9]*", GROUP="root", MODE="0666"' >>  /etc/udev/rules.d/81-ddfile.rules
# 	exit

load:
	sudo insmod DDFile.ko 
# && sudo mknod --mode=666 /dev/ddfile2 c `grep DDFile /proc/devices | awk '{print $1;}'` 0

unload:
	sudo rmmod DDFile
# && sudo rm /dev/ddfile2

write:
	echo "a" > /dev/ddfile0

read:
	echo `cat /dev/ddfile0`

check_device_file:
	ls -al /dev/dd*

check_device_module:
	echo `cat /proc/devices | grep DDFile && ls -la /dev | grep ddfile` \
	&& echo `cat /sys/class/ddfile/ddfile0/dev` \
	&& echo `cat /sys/class/ddfile/ddfile0/uevent`

check_message:
	sudo dmesg | grep ddfile

build_test:
	gcc test.c -o tester

run_test:
	./tester
