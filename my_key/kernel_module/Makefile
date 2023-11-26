obj-m :=key.o

KDIR :=/home/ren/loongarch/linux-5.10-2k500-cbd-src

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers .*.cmd *.mod modules.order

