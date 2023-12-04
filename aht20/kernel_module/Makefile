# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Jinrun Yang, Junchi Ren

#obj-m +=aht20.o
#obj-m +=aht20_dev.o
obj-m +=aht20_i2c.o

KDIR :=${MY_KDIR}

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) modules clean

