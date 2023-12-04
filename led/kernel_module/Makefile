# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Junchi Ren, Jinrun Yang

obj-m :=led_drv.o

KDIR :=${MY_KDIR}

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) modules clean

