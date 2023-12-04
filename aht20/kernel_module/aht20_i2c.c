// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEVICE_NAME		"aht20"	/* 名字 	*/
#define DEVICE_MINOR		147			/* 子设备号 */

#define DEVICE_ADDR	0x38
#define DEVICE_WRITE_ADDR	((DEVICE_ADDR << 1) | 0)
#define DEVICE_READ_ADDR	((DEVICE_ADDR << 1) | 1)

#define SDA_GPIO                68
#define SCL_GPIO                69
//GPIO口在当前32位寄存器中的对应位
#define SDA_GPIO_INDEX		(u32)(1 << (SDA_GPIO % 32))
#define SCL_GPIO_INDEX		(u32)(1 << (SCL_GPIO % 32))
#define DELAY_TIME          	10

#define GPCFG	0x1fe104b0

/* __iomem  虚拟地址指针，保存虚拟地址*/
static void __iomem *GPCFG_V;

struct aht20_dev {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int sda_gpio;
        int scl_gpio;
	unsigned char write_addr;
	unsigned char read_addr;
};

struct aht20_dev my_dev;

static void SDA_D_OUT(void)
{
	gpio_direction_output(SDA_GPIO, 1);
}

static void SDA_D_IN(void)
{
	gpio_direction_input(SDA_GPIO);
}

static void SCL_SET(void)
{
    gpio_set_value(my_dev.scl_gpio, 1);
}

static void SCL_CLR(void)
{
     gpio_set_value(my_dev.scl_gpio, 0);
}

static void SDA_SET(void)
{
    gpio_set_value(my_dev.sda_gpio, 1);
}

static void SDA_CLR(void)
{
    gpio_set_value(my_dev.sda_gpio, 0);
}

static int SDA_READ(void)
{
	return gpio_get_value(my_dev.sda_gpio);
}

void I2c_Start( void )    // 操作起始
{
	SDA_D_OUT();    // 设置SDA为输出方向 
	SDA_SET();    // 发送起始条件的数据信号
 	SCL_SET();
    	udelay(DELAY_TIME);
    	SDA_CLR();    //发送起始信号
    	udelay(DELAY_TIME);      
    	SCL_CLR();    //钳住I2C总线，准备发送或接收数据 
}

void I2c_Stop( void )    // 操作结束
{
	SDA_D_OUT();    // 设置SDA为输出方向 
    	SDA_CLR();
	SCL_CLR();
    	udelay(DELAY_TIME);
    	SCL_SET();
    	SDA_SET();    // 发送I2C总线结束信号
    	udelay(DELAY_TIME);
}

void I2c_Ack( void )
{
	SCL_CLR();
	SDA_D_OUT();
	SDA_CLR();
	udelay(DELAY_TIME);
	SCL_SET();
	udelay(DELAY_TIME);
	SCL_CLR();
}

void I2c_NAck( void )
{
	SCL_CLR();
	SDA_D_OUT();
	SDA_SET();
	udelay(DELAY_TIME);
	SCL_SET();
	udelay(DELAY_TIME);
	SCL_CLR();
}

void I2c_WrByte( unsigned char dat )    // 写一个字节数据
{
  	unsigned char i;
	SDA_D_OUT();
	SCL_CLR();
  	for( i = 0; i != 8; i++ )    // 输出8位数据
    	{
        	if( dat & 0x80 )
		{
			SDA_SET();
		}
        	else 
		{ 
			SDA_CLR(); 
		}
		dat <<= 1;
        	udelay(DELAY_TIME);
        	SCL_SET();
        	udelay(DELAY_TIME);    // 可选延时
        	SCL_CLR();
    	}
    	SDA_SET();
    	udelay(DELAY_TIME);
    	SCL_SET();    // 接收应答
    	udelay(DELAY_TIME);
    	SCL_CLR();
}

unsigned char I2c_RdByte( int ack )
{
	unsigned char i, ret = 0;
	SDA_D_IN();
	for (i = 0; i < 8; i++)
	{
		SCL_CLR();
		udelay(DELAY_TIME);
		SCL_SET();
		ret <<= 1;
		if (SDA_READ())
		{
			ret++;
		}
		udelay(DELAY_TIME);
	}
	if (ack)
	{
		I2c_Ack();
	}
	else
	{
		I2c_NAck();
	}
	return ret;
}

void AHT20_SendCmd( unsigned char *cmd_buf, size_t cnt )
{
	int i;
   	I2c_Start();    // 启动总线
	I2c_WrByte(my_dev.write_addr);
	for (i = 0; i < cnt; i++)
	{
		I2c_WrByte( ( unsigned char )( cmd_buf[i] ) );
	} 
	I2c_Stop();    // 结束总线  
}

void AHT20_Recv( unsigned char *buf, size_t cnt )
{
	int i;
	I2c_Start();
	I2c_WrByte(my_dev.read_addr);
	for (i = 0; i < cnt; i++)
	{
		buf[i] = I2c_RdByte(cnt - i - 1);
	}
	I2c_Stop();
}

static int aht20_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &my_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t aht20_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	unsigned char databuf[7], cmd[3];
	
	mdelay(50);
	
	cmd[0] = 0xac;
	cmd[1] = 0x33;
	cmd[2] = 0x00;
	AHT20_SendCmd(cmd, 3);

	mdelay(80);

	AHT20_Recv(databuf, 7);

        return copy_to_user(buf, databuf, cnt);
}

static int aht20_release(struct inode *inode, struct file *filp)
{
	return 0;
}


/* 设备操作函数 */
static struct file_operations aht20_ops = {
	.owner = THIS_MODULE,
	.open = aht20_open,
        .read = aht20_read,
	.release = aht20_release,
};

/* MISC设备结构体 */
static struct miscdevice aht20_miscdev = {
	.minor = DEVICE_MINOR,
	.name = DEVICE_NAME,
	.fops = &aht20_ops,
};


static int __init aht20_init(void)
{
        int ret;
        u32 GPCFG_read, GPCFG_write;

        ret = misc_register(&aht20_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

        GPCFG_V = ioremap(GPCFG, sizeof(u32));
        GPCFG_read = ioread32(GPCFG_V);
        GPCFG_write = (GPCFG_read & 0x77007777);
        iowrite32(GPCFG_write, GPCFG_V);

        my_dev.sda_gpio = SDA_GPIO;
        my_dev.scl_gpio = SCL_GPIO;
	my_dev.write_addr = DEVICE_WRITE_ADDR;
	my_dev.read_addr = DEVICE_READ_ADDR;

        gpio_request(my_dev.sda_gpio, "sda3");
        gpio_request(my_dev.scl_gpio, "scl3");
        ret = gpio_direction_output(my_dev.sda_gpio, 1);
	if(ret < 0) {
		printk("can't set sda gpio!\r\n");
	}    
 
	ret = gpio_direction_output(my_dev.scl_gpio, 1);
	if(ret < 0) {
		printk("can't set scl gpio!\r\n");
	}
	return ret;
}

static void __exit aht20_exit(void)
{
        iounmap(GPCFG_V);

       	misc_deregister(&aht20_miscdev);
       	gpio_free(my_dev.sda_gpio);
       	gpio_free(my_dev.scl_gpio);
}

module_init(aht20_init);
module_exit(aht20_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

