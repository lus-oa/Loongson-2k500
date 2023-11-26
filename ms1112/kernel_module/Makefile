obj-m :=ms1112_i2c.o

KDIR :=/root/loongarch/linux-5.10-2k500-cbd-src

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) modules clean

