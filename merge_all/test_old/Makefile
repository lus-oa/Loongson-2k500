CC =loongarch64-linux-gnu-gcc
normal_target =test_aht20.o
normal_target +=test_beep.o
normal_target +=test_stk8ba.o
normal_target +=test_motor.o
normal_target +=test_rc522.o
thread_target +=test_ch422g.o
target +=$(normal_target)
target +=$(thread_target)

all: $(target)
$(normal_target):%.o:%.c
	$(CC) -O2 $^ -o $(patsubst %.o,%,$@)
$(thread_target):%.o:%.c
	$(CC) -lpthread -O2 $^ -o $(patsubst %.o,%,$@)

clean:
	rm $(patsubst %.o,%,$(target))

