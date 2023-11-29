#obj-m +=aht20.o
#obj-m +=aht20_dev.o
obj-m +=aht20_i2c.o

KDIR :=${MY_KDIR}

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) modules clean

