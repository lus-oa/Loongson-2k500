// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include "I2c_Pins.h"

#define GPIO_INDEX_32(gpio)	(u32)(1 << (gpio % 32))
#define I2C_DELAY_TIME		10

#define GPCFG_3		0x1fe104b0	//I2C3 0x00770000
#define GPCFG_CH422	0x1fe104d4	//CH422G 0x07700000
static void __iomem *GPCFG_V;

static int __init softi2c_init(void)
{
	int ret = 0;
	u32 GPCFG_read, GPCFG_write;

	printk("softi2c api init.\n");

        GPCFG_V = ioremap(GPCFG_3, sizeof(u32));
        GPCFG_read = ioread32(GPCFG_V);
        GPCFG_write = (GPCFG_read & 0x77007777);
        iowrite32(GPCFG_write, GPCFG_V);
	iounmap(GPCFG_V);
	
	GPCFG_V = ioremap(GPCFG_CH422, sizeof(u32));
        GPCFG_read = ioread32(GPCFG_V);
        GPCFG_write = (GPCFG_read & 0x70077777);
	iowrite32(GPCFG_write, GPCFG_V);
	iounmap(GPCFG_V);

	gpio_request(SDA3_GPIO, "sda3");
        gpio_request(SCL3_GPIO, "scl3");
	ret = gpio_direction_output(SDA3_GPIO, 1);
	if(ret < 0) {
		printk("can't set sda3 gpio!\r\n");
	}
	ret = gpio_direction_output(SCL3_GPIO, 1);
	if(ret < 0) {
		printk("can't set scl3 gpio!\r\n");
	}
	
	gpio_request(CH422G_SDA_GPIO, "ch422g_sda");
        gpio_request(CH422G_SCL_GPIO, "ch422g_scl");
        ret = gpio_direction_output(CH422G_SDA_GPIO, 0);
	if(ret < 0) {
		printk("can't set ch422g sda gpio!\r\n");
	}    
        ret = gpio_direction_output(CH422G_SCL_GPIO, 0);
	if(ret < 0) {
		printk("can't set ch422g scl gpio!\r\n");
	}	

	return 0;
}

static void __exit softi2c_exit(void)
{
	printk("softi2c api exit.\n");
	gpio_free(SDA3_GPIO);
	gpio_free(SCL3_GPIO);
	gpio_free(CH422G_SDA_GPIO);
        gpio_free(CH422G_SCL_GPIO);
}

module_init(softi2c_init);
module_exit(softi2c_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");
MODULE_ALIAS("SoftI2c_API");

DEFINE_SPINLOCK(softi2c3_lock);

void get_i2c_lock(I2c_Pins *pins)
{
	if (pins->sda == SDA3_GPIO && pins->scl == SCL3_GPIO)
	{
		spin_lock(&softi2c3_lock);
	}
	else
	{
		printk("i2c spinlock for %d,%d doesn't exist\n"\
			, pins->sda, pins->scl);
	}
}
EXPORT_SYMBOL(get_i2c_lock);

void free_i2c_lock(I2c_Pins *pins)
{
	if (pins->sda == SDA3_GPIO && pins->scl == SCL3_GPIO)
	{
		spin_unlock(&softi2c3_lock);
	}
	else
	{
		printk("i2c spinlock for %d,%d doesn't exist\n"\
			, pins->sda, pins->scl);
	}
}
EXPORT_SYMBOL(free_i2c_lock);

static inline void SDA_D_OUT(I2c_Pins *pins)
{
	gpio_direction_output(pins->sda, 1);
}

static inline void SDA_D_IN(I2c_Pins *pins)
{
	gpio_direction_input(pins->sda);
}

static inline void SCL_SET(I2c_Pins *pins)
{
	gpio_set_value(pins->scl, 1);
}

static inline void SCL_CLR(I2c_Pins *pins)
{
	gpio_set_value(pins->scl, 0);
}

static inline void SDA_SET(I2c_Pins *pins)
{
	gpio_set_value(pins->sda, 1);
}

static inline void SDA_CLR(I2c_Pins *pins)
{
	gpio_set_value(pins->sda, 0);
}

static inline int SDA_READ(I2c_Pins *pins)
{
	return gpio_get_value(pins->sda);
}

void I2c_Start( I2c_Pins *pins )    // 操作起始
{
	SDA_D_OUT(pins);    // 设置SDA为输出方向 
	SDA_SET(pins);    // 发送起始条件的数据信号
 	SCL_SET(pins);
    	udelay(I2C_DELAY_TIME);
    	SDA_CLR(pins);    //发送起始信号
    	udelay(I2C_DELAY_TIME);      
    	SCL_CLR(pins);    //钳住I2C总线，准备发送或接收数据 
}
EXPORT_SYMBOL(I2c_Start);

void I2c_Stop( I2c_Pins *pins )    // 操作结束
{
	SDA_D_OUT(pins);    // 设置SDA为输出方向 
    	SDA_CLR(pins);
	SCL_CLR(pins);
    	udelay(I2C_DELAY_TIME);
    	SCL_SET(pins);
    	SDA_SET(pins);    // 发送I2C总线结束信号
    	udelay(I2C_DELAY_TIME);
}
EXPORT_SYMBOL(I2c_Stop);

static void I2c_Ack( I2c_Pins *pins )
{
	SCL_CLR(pins);
	SDA_D_OUT(pins);
	SDA_CLR(pins);
	udelay(I2C_DELAY_TIME);
	SCL_SET(pins);
	udelay(I2C_DELAY_TIME);
	SCL_CLR(pins);
}

static void I2c_NAck( I2c_Pins *pins )
{
	SCL_CLR(pins);
	SDA_D_OUT(pins);
	SDA_SET(pins);
	udelay(I2C_DELAY_TIME);
	SCL_SET(pins);
	udelay(I2C_DELAY_TIME);
	SCL_CLR(pins);
}

void I2c_WrByte( I2c_Pins *pins, unsigned char dat )    // 写一个字节数据
{
  	unsigned char i;
	SDA_D_OUT(pins);
	SCL_CLR(pins);
  	for( i = 0; i != 8; i++ )    // 输出8位数据
    	{
        	if( dat & 0x80 )
		{
			SDA_SET(pins);
		}
        	else 
		{ 
			SDA_CLR(pins); 
		}
		dat <<= 1;
        	udelay(I2C_DELAY_TIME);
        	SCL_SET(pins);
        	udelay(I2C_DELAY_TIME);    // 可选延时
        	SCL_CLR(pins);
    	}
    	SDA_SET(pins);
    	udelay(I2C_DELAY_TIME);
    	SCL_SET(pins);    // 接收应答
    	udelay(I2C_DELAY_TIME);
    	SCL_CLR(pins);
}
EXPORT_SYMBOL(I2c_WrByte);

unsigned char I2c_RdByte( I2c_Pins *pins, int ack )
{
	unsigned char i, ret = 0;
	SDA_D_IN(pins);
	for (i = 0; i < 8; i++)
	{
		SCL_CLR(pins);
		udelay(I2C_DELAY_TIME);
		SCL_SET(pins);
		ret <<= 1;
		if (SDA_READ(pins))
		{
			ret++;
		}
		udelay(I2C_DELAY_TIME);
	}
	if (ack)
	{
		I2c_Ack(pins);
	}
	else
	{
		I2c_NAck(pins);
	}
	return ret;
}
EXPORT_SYMBOL(I2c_RdByte);

