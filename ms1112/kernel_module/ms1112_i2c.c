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

#define DEVICE_NAME		"adc"	/* 名字 	*/
#define DEVICE_MINOR		152			/* 子设备号 */

#define DEVICE_ADDR	0x4E
#define DEVICE_WRITE_ADDR	((DEVICE_ADDR << 1) | 0)
#define DEVICE_READ_ADDR	((DEVICE_ADDR << 1) | 1)

#define SDA_GPIO                68
#define SCL_GPIO                69
//GPIO口在当前32位寄存器中的对应位
#define SDA_GPIO_INDEX		(u32)(1 << (SDA_GPIO % 32))
#define SCL_GPIO_INDEX		(u32)(1 << (SCL_GPIO % 32))
#define DELAY_TIME          	10
#define ACK	1
#define NACK	0

#define GPCFG	0x1fe104b0	//68 69复用配置寄存器
//0-31位对应GPIO64-95的输出使能寄存器，0输出1输入
#define GPIOE	0x1fe10450

/* __iomem  虚拟地址指针，保存虚拟地址*/
static void __iomem *GPCFG_V;
static void __iomem *GPIOE_V;

struct ms1112_dev {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int sda_gpio;
        int scl_gpio;
	unsigned char write_addr;
	unsigned char read_addr;
};

struct ms1112_dev my_dev;

static void SDA_D_OUT(void)
{
	u32 GPIOE_read, GPIOE_write;
	GPIOE_read = ioread32(GPIOE_V);
	GPIOE_write = (GPIOE_read & (~SDA_GPIO_INDEX));
	iowrite32(GPIOE_write, GPIOE_V);
}

static void SDA_D_IN(void)
{
	u32 GPIOE_read, GPIOE_write;
	GPIOE_read = ioread32(GPIOE_V);
	GPIOE_write = (GPIOE_read | SDA_GPIO_INDEX);
	iowrite32(GPIOE_write, GPIOE_V);
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

//MS1112只能写入配置寄存器
void MS1112_Write_Reg( unsigned char new_value )
{
   	I2c_Start();    // 启动总线

	I2c_WrByte(my_dev.write_addr);
	I2c_WrByte(new_value);

	I2c_Stop();    // 结束总线  
}

//MS1112只有一种读操作，读取3个字节--输出寄存器2字节+配置寄存器1字节
static void MS1112_Read_Reg(unsigned char *buf)
{
	I2c_Start();
	I2c_WrByte(my_dev.read_addr);
	buf[0] = I2c_RdByte(ACK);
	buf[1] = I2c_RdByte(ACK);
	buf[2] = I2c_RdByte(NACK);
	I2c_Stop();
}

static int ms1112_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &my_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t ms1112_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	unsigned char data[3];
	struct stk8ba_dev *dev = (struct stk8ba_dev *)filp->private_data;
	
	MS1112_Read_Reg(data);

        return copy_to_user(buf, data, sizeof(data));
}

static ssize_t ms1112_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
        int ret;
	unsigned char databuf[1];
	struct stk8ba_dev *dev = filp->private_data;

	ret = copy_from_user(databuf, buf, cnt);
	if(ret < 0) {
		pr_info("kernel write failed!\r\n");
		return -EFAULT;
	}

        MS1112_Write_Reg(databuf[0]);

        return 0;
}

static int ms1112_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations ms1112_ops = {
	.owner = THIS_MODULE,
	.open = ms1112_open,
        .read = ms1112_read,
	.write = ms1112_write,
	.release = ms1112_release,
};

/* MISC设备结构体 */
static struct miscdevice ms1112_miscdev = {
	.minor = DEVICE_MINOR,
	.name = DEVICE_NAME,
	.fops = &ms1112_ops,
};


static int __init ms1112_init(void)
{
        int ret;
	u32 GPCFG_read, GPCFG_write;
	
	ret = misc_register(&ms1112_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

	GPIOE_V = ioremap(GPIOE, sizeof(u32));

	GPCFG_V = ioremap(GPCFG, sizeof(u32));
        GPCFG_read = ioread32(GPCFG_V);
        GPCFG_write = (GPCFG_read & 0x77007777);
        iowrite32(GPCFG_write, GPCFG_V);

        gpio_request(SDA_GPIO, "sda3");
        gpio_request(SCL_GPIO, "scl3");
	
	my_dev.sda_gpio = SDA_GPIO;
        my_dev.scl_gpio = SCL_GPIO;
	my_dev.write_addr = DEVICE_WRITE_ADDR;
	my_dev.read_addr = DEVICE_READ_ADDR;
	
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

static void __exit ms1112_exit(void)
{

        iounmap(GPCFG_V);
        iounmap(GPIOE_V);

       	misc_deregister(&ms1112_miscdev);
       	gpio_free(my_dev.sda_gpio);
       	gpio_free(my_dev.scl_gpio);
}

module_init(ms1112_init);
module_exit(ms1112_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

