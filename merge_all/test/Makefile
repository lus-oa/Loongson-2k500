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

opath =$(patsubst %,$(OBJ)/%,$(objs))
target =app

all: $(target)
$(target):$(opath)
	$(CC) -O2 $^ -o $@ $(INC)
$(opath):$(OBJ)/%.o:$(SRC)/%.c
	$(CC) -O2 -lpthread $^ -c -o $@ $(INC)

clean:
	rm $(target) $(opath)

