#ifndef __SOFTI2C_H__
#define __SOFTI2C_H__

#include <linux/gpio.h>
#include <linux/spinlock.h>

//引入I2c_Pins结构体及GPIO
#include "I2c_Pins.h"

#define I2C_WRITE_ADDR(addr)	((addr << 1) | 0)
#define I2C_READ_ADDR(addr)	((addr << 1) | 1)

//i2c锁--自旋锁
//TODO 可以考虑换互斥锁，根据不同I2C总线加锁
extern spinlock_t softi2c_lock;
#define get_i2c_lock()		spin_lock(&softi2c_lock)
#define free_i2c_lock()		spin_unlock(&softi2c_lock)

#define ACK	1
#define NACK	0

extern void I2c_Start( I2c_Pins *pins );
extern void I2c_Stop( I2c_Pins *pins );
extern void I2c_WrByte( I2c_Pins *pins, unsigned char dat );
extern unsigned char I2c_RdByte( I2c_Pins *pins, int ack );

#endif
