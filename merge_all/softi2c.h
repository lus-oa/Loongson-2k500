// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#ifndef __SOFTI2C_H__
#define __SOFTI2C_H__

#include <linux/gpio.h>
#include <linux/spinlock.h>

//引入I2c_Pins结构体及GPIO
#include "I2c_Pins.h"

#define I2C_WRITE_ADDR(addr)	((addr << 1) | 0)
#define I2C_READ_ADDR(addr)	((addr << 1) | 1)

//i2c锁--自旋锁
extern void get_i2c_lock(I2c_Pins *pins);
extern void free_i2c_lock(I2c_Pins *pins);

#define ACK	1
#define NACK	0

extern void I2c_Start(I2c_Pins *pins);
extern void I2c_Stop(I2c_Pins *pins);
extern void I2c_WrByte(I2c_Pins *pins, unsigned char dat);
extern unsigned char I2c_RdByte(I2c_Pins *pins, int ack);

#endif

