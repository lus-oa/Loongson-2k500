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

#define DEVICE_NAME		"sd2068"	/* 名字 	*/
#define DEVICE_MINOR		188			/* 子设备号 */

#define DEVICE_ADDR	0x32
#define DEVICE_WRITE_ADDR	((DEVICE_ADDR << 1) | 0)
#define DEVICE_READ_ADDR	((DEVICE_ADDR << 1) | 1)

#define SD2068_SEC		0x00
#define SD2068_MIN		0x01
#define SD2068_HOUR		0x02
#define SD2068_WEEK		0x03
#define SD2068_DAY		0x04
#define SD2068_MON		0x05
#define SD2068_YEAR		0x06
#define SD2068_CTR1		0x0F
#define SD2068_CTR2		0x10
#define SD2068_CTR3		0x11

#define SDA_GPIO                65
#define SCL_GPIO                64
//GPIO口在当前32位寄存器中的对应位
#define SDA_GPIO_INDEX		(u32)(1 << (SDA_GPIO % 32))
#define SCL_GPIO_INDEX		(u32)(1 << (SCL_GPIO % 32))
#define DELAY_TIME          	10
#define ACK	1
#define NACK	0

#define GPCFG	0x1fe104b0	//GPIO复用配置寄存器

/* __iomem  虚拟地址指针，保存虚拟地址*/
static void __iomem *GPCFG_V;

struct sd2068_dev {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int sda_gpio;
        int scl_gpio;
	unsigned char write_addr;
	unsigned char read_addr;
};

struct sd2068_dev my_dev;

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

static void SD2068_Write_Reg( unsigned char reg_addr, unsigned char new_value )
{
   	I2c_Start();    // 启动总线

	I2c_WrByte(my_dev.write_addr);
	I2c_WrByte(reg_addr);
	I2c_WrByte(new_value);

	I2c_Stop();    // 结束总线  

	//pr_info("sd2068 write reg %x done, new value: %x\n", reg_addr, new_value);
}

static unsigned char SD2068_Read_Reg(unsigned char reg)
{
	unsigned char val;

	I2c_Start();
	I2c_WrByte(my_dev.write_addr);
	I2c_WrByte(reg);
	//I2c_Stop();
	
	I2c_Start();	//restart
	I2c_WrByte(my_dev.read_addr);
	val = I2c_RdByte(NACK);
	I2c_Stop();

	//pr_info("sd2068 read reg %x done, value: %x\n", reg, val);
	
	return val;
}

static void SD2068_Write_Enable( void )
{
	u8 ctr1, ctr2;
	//开启写使能，WRTC1先置1，再将WRTC23置1
	ctr1 = SD2068_Read_Reg(SD2068_CTR1);
	ctr2 = SD2068_Read_Reg(SD2068_CTR2);
	SD2068_Write_Reg(SD2068_CTR2, (ctr2 | 0x80));
	SD2068_Write_Reg(SD2068_CTR1, (ctr1 | 0x84));
}

static void SD2068_Write_Disable( void )
{
	u8 ctr1, ctr2;
	//关闭写使能，WRTC23先置0，再将WRTC1置0
	ctr1 = SD2068_Read_Reg(SD2068_CTR1);
	ctr2 = SD2068_Read_Reg(SD2068_CTR2);
	SD2068_Write_Reg(SD2068_CTR1, (ctr1 & ~(0x84)));
	SD2068_Write_Reg(SD2068_CTR2, (ctr2 & ~(0x80)));
}

static int sd2068_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &my_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t sd2068_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	unsigned char time_buf[7];

	struct sd2068_dev *dev = filp->private_data;
	
	time_buf[0] = SD2068_Read_Reg(SD2068_SEC);
       	time_buf[1] = SD2068_Read_Reg(SD2068_MIN);
       	time_buf[2] = SD2068_Read_Reg(SD2068_HOUR);
       	time_buf[3] = SD2068_Read_Reg(SD2068_WEEK);
       	time_buf[4] = SD2068_Read_Reg(SD2068_DAY);
       	time_buf[5] = SD2068_Read_Reg(SD2068_MON);
       	time_buf[6] = SD2068_Read_Reg(SD2068_YEAR);

        return copy_to_user(buf, time_buf, sizeof(time_buf));
}

static ssize_t sd2068_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
        int ret;
	unsigned char databuf[7];
	struct sd2068_dev *dev = filp->private_data;

	ret = copy_from_user(databuf, buf, cnt);
	if(ret < 0) {
		pr_info("kernel write failed!\r\n");
		return -EFAULT;
	}

	SD2068_Write_Enable();

	//约定用户传入7字节BCD码
        SD2068_Write_Reg(SD2068_SEC, databuf[0]);
        SD2068_Write_Reg(SD2068_MIN, databuf[1]);
        SD2068_Write_Reg(SD2068_HOUR, databuf[2]);
        SD2068_Write_Reg(SD2068_WEEK, databuf[3]);
        SD2068_Write_Reg(SD2068_DAY, databuf[4]);
        SD2068_Write_Reg(SD2068_MON, databuf[5]);
        SD2068_Write_Reg(SD2068_YEAR, databuf[6]);

	SD2068_Write_Disable();

        return 0;
}

static int sd2068_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations sd2068_ops = {
	.owner = THIS_MODULE,
	.open = sd2068_open,
        .read = sd2068_read,
	.write = sd2068_write,
	.release = sd2068_release,
};

/* MISC设备结构体 */
static struct miscdevice sd2068_miscdev = {
	.minor = DEVICE_MINOR,
	.name = DEVICE_NAME,
	.fops = &sd2068_ops,
};


static int __init sd2068_init(void)
{
        int ret;
	u32 GPCFG_read, GPCFG_write;
	u8 ctr1, ctr2;
	
	ret = misc_register(&sd2068_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

	GPCFG_V = ioremap(GPCFG, sizeof(u32));
	GPCFG_read = ioread32(GPCFG_V);
	GPCFG_write = (GPCFG_read & 0x77777700);
	iowrite32(GPCFG_write, GPCFG_V);

        gpio_request(SDA_GPIO, "sda1");
        gpio_request(SCL_GPIO, "scl1");
	
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

static void __exit sd2068_exit(void)
{

        iounmap(GPCFG_V);

       	misc_deregister(&sd2068_miscdev);
       	gpio_free(my_dev.sda_gpio);
       	gpio_free(my_dev.scl_gpio);
}

module_init(sd2068_init);
module_exit(sd2068_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

