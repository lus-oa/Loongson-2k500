# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Jinrun Yang, Junchi Ren

CC =loongarch64-linux-gnu-gcc
INC =-I ./inc
SRC =./src
OBJ =./obj
objs +=main.o
objs +=aht20.o
objs +=beep.o
objs +=motor.o
objs +=stk8ba.o
objs +=aip1944.o
objs +=rc522.o
objs +=ch422g.o
objs +=key16.o
objs +=led.o
objs +=ir.o

opath =$(patsubst %,$(OBJ)/%,$(objs))
target =app

all: $(target)
$(target):$(opath)
	$(CC) -O2 -lpthread $^ -o $@ $(INC)
$(opath):$(OBJ)/%.o:$(SRC)/%.c
	$(CC) -O2 $^ -c -o $@ $(INC)

clean:
	rm $(target) $(opath)

