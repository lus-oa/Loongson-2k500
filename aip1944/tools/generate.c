// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

#include <stdio.h>

__uint8_t bitmap[] = {
    0x10, 0x20, 
    0x10, 0x20, 
    0x10, 0x20, 
    0x13, 0xfe, 
    0xfc, 0x20, 
    0x10, 0x20, 
    0x10, 0x20, 
    0x15, 0xfc, 
    0x18, 0x84, 
    0x30, 0x88, 
    0xd0, 0x48, 
    0x10, 0x50, 
    0x10, 0x20, 
    0x10, 0x50, 
    0x51, 0x88, 
    0x26, 0x06
};

__uint8_t correct[32] = {};

__uint8_t step(__uint8_t origin)
{
	__uint8_t tmp = 0;
	tmp = ((origin << 4) & 0xf0) | ((origin & 0xf0) >> 4);
	tmp = ((tmp << 2) & 0xCC) | ((tmp & 0xCC) >> 2);
	tmp = ((tmp << 1) & 0xAA) | ((tmp & 0xAA) >> 1);
	return tmp;
}

void transform()
{
	for (int i = 0; i < sizeof(bitmap) / sizeof(__uint8_t); i++) {
		correct[i] = step(bitmap[i]);
	}
}

void output()
{
	for (int i = 0; i < sizeof(bitmap) / sizeof(__uint8_t); i++) {
		printf(" 0x%X,", correct[i]);
	}
	printf("\n");
}

int main()
{
	transform();
	output();
}