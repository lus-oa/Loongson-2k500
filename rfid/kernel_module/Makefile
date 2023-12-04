# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Jinrun Yang, Junchi Ren

obj-m :=RFID-RC522.o
RFID-RC522-objs:=rc522.o rc522_api.o

#EXTRA_CFLAGS = 

KDIR :=${MY_KDIR}

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) modules clean

