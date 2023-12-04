# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Jinrun Yang, Junchi Ren

obj-m +=softi2c.o
obj-m +=aht20.o
obj-m += beep.o
obj-m += ch422g.o
obj-m += motor.o
obj-m += rfid.o
rfid-objs := rc522.o rc522_api.o
obj-m += stk8ba.o
obj-m += aip1944.o
obj-m += led.o
obj-m += key.o
obj-m += irdrv.o


KDIR :=$(MY_KDIR)

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean

