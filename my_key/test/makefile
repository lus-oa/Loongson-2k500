# Copyright (c) 2023 Shandong University
# Copyright (c) 2023 Junchi Ren, Jinrun Yang

all: test 
clean: 
	rm -rf test_key  key_scan
test: 
	loongarch64-linux-gnu-gcc -O2 test_key.c -o test_key 
	loongarch64-linux-gnu-gcc -O2 key_scan.c -o key_scan 

