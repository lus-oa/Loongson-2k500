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
#include "stk8ba.h"

#define DEVICE_NAME		"stk8ba"	/* 名字 	*/
#define DEVICE_MINOR		148			/* 子设备号 */

#define DEVICE_ADDR	0x18
#define DEVICE_WRITE_ADDR	((DEVICE_ADDR << 1) | 0)
#define DEVICE_READ_ADDR	((DEVICE_ADDR << 1) | 1)

#define DELAY_TIME          	10
#define ACK	1
#define NACK	0

#define GPCFG1	0x1fe104c0	//INT管脚复用配置寄存器
#define GPCFG2	0x1fe104b0	//68 69复用配置寄存器

/* __iomem  虚拟地址指针，保存虚拟地址*/
static void __iomem *GPCFG1_V;
static void __iomem *GPCFG2_V;

struct stk8ba_dev {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int sda_gpio;
        int scl_gpio;
	int irq_num;
	unsigned char write_addr;
	unsigned char read_addr;
};

struct stk8ba_dev my_dev;

static void SDA_D_OUT(void)
{
	gpio_direction_output(my_dev.sda_gpio, 1);
}

static void SDA_D_IN(void)
{
	gpio_direction_input(my_dev.sda_gpio);
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

void STK8BA_Write_Reg( unsigned char reg_addr, unsigned char new_value )
{
   	I2c_Start();    // 启动总线

	I2c_WrByte(my_dev.write_addr);
	I2c_WrByte(reg_addr);
	I2c_WrByte(new_value);

	I2c_Stop();    // 结束总线  
}

static unsigned char STK8BA_Read_Reg(unsigned char reg)
{
	unsigned char val;

	I2c_Start();
	I2c_WrByte(my_dev.write_addr);
	I2c_WrByte(reg);
	I2c_Stop();
	
	I2c_Start();
	I2c_WrByte(my_dev.read_addr);
	val = I2c_RdByte(NACK);
	I2c_Stop();
	
	return val;
}

static void stk8ba_read_xyz(unsigned char *data)
{
	data[0] = STK8BA_Read_Reg(STK8BA_XOUT1);
        data[1] = STK8BA_Read_Reg(STK8BA_XOUT2);
        data[2] = STK8BA_Read_Reg(STK8BA_YOUT1);
        data[3] = STK8BA_Read_Reg(STK8BA_YOUT2);
        data[4] = STK8BA_Read_Reg(STK8BA_ZOUT1);
        data[5] = STK8BA_Read_Reg(STK8BA_ZOUT2);
}

static int stk8ba_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &my_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t stk8ba_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	unsigned char data[6];

	struct stk8ba_dev *dev = (struct stk8ba_dev *)filp->private_data;
	
	stk8ba_read_xyz(data);

        return copy_to_user(buf, data, cnt);
}

static ssize_t stk8ba_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
        int ret;
	unsigned char databuf[2];
	struct stk8ba_dev *dev = filp->private_data;

	ret = copy_from_user(databuf, buf, cnt);
	if(ret < 0) {
		pr_info("kernel write failed!\r\n");
		return -EFAULT;
	}

        STK8BA_Write_Reg(databuf[0], databuf[1]);

        return 0;
}

static int stk8ba_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int prev_xyz[3] = { 0 };
static unsigned char data_buf[6];
static irqreturn_t read_handler(int irq, void *dev_id)
{
	//struct stk8ba_dev *dev = (struct stk8ba_dev *)dev_id;

	return IRQ_RETVAL(IRQ_HANDLED);
}

/* 设备操作函数 */
static struct file_operations stk8ba_ops = {
	.owner = THIS_MODULE,
	.open = stk8ba_open,
        .read = stk8ba_read,
	.write = stk8ba_write,
	.release = stk8ba_release,
};

/* MISC设备结构体 */
static struct miscdevice stk8ba_miscdev = {
	.minor = DEVICE_MINOR,
	.name = DEVICE_NAME,
	.fops = &stk8ba_ops,
};


static int __init stk8ba_init(void)
{
        int ret;
	u32 GPCFG1_read, GPCFG1_write;
        u32 GPCFG2_read, GPCFG2_write;
	
	ret = misc_register(&stk8ba_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

	GPCFG1_V = ioremap(GPCFG1, sizeof(u32));
	GPCFG1_read = ioread32(GPCFG1_V);
	GPCFG1_write = (GPCFG1_read & 0x77707777);
	iowrite32(GPCFG1_write, GPCFG1_V);

	GPCFG2_V = ioremap(GPCFG2, sizeof(u32));
        GPCFG2_read = ioread32(GPCFG2_V);
        GPCFG2_write = (GPCFG2_read & 0x77007777);
        iowrite32(GPCFG2_write, GPCFG2_V);

        gpio_request(SDA_GPIO, "stk8ba_sda");
        gpio_request(SCL_GPIO, "stk8ba_scl");
	gpio_request(INT_GPIO, "stk8ba_int");
	
	my_dev.sda_gpio = SDA_GPIO;
        my_dev.scl_gpio = SCL_GPIO;
	my_dev.irq_num = gpio_to_irq(INT_GPIO);
	my_dev.write_addr = DEVICE_WRITE_ADDR;
	my_dev.read_addr = DEVICE_READ_ADDR;
	pr_info("stk8ba_init gpio int irq num:%d\n",my_dev.irq_num);
	
        ret = gpio_direction_output(my_dev.sda_gpio, 1);
	if(ret < 0) {
		printk("can't set sda gpio!\r\n");
	}    
 
	ret = gpio_direction_output(my_dev.scl_gpio, 1);
	if(ret < 0) {
		printk("can't set scl gpio!\r\n");
	}

	pr_info("id:%x\n", STK8BA_Read_Reg(STK8BA_CHIP_ID));

	STK8BA_Write_Reg(STK8BA_INTMAP1, 0x5);
        STK8BA_Write_Reg(STK8BA_INTMAP2, 0x1);

        STK8BA_Write_Reg(STK8BA_INTEN1, 0x7);
        STK8BA_Write_Reg(STK8BA_INTEN2, 0x10);

	ret = request_irq(my_dev.irq_num, read_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "stk8ba-irq", &my_dev);

	pr_info("intcfg1:%x, intcfg2:%x\n", STK8BA_Read_Reg(STK8BA_INTCFG1), STK8BA_Read_Reg(STK8BA_INTCFG2));
  
        return ret;
}

static void __exit stk8ba_exit(void)
{

        iounmap(GPCFG1_V);
	iounmap(GPCFG2_V);

       	misc_deregister(&stk8ba_miscdev);
       	gpio_free(my_dev.sda_gpio);
       	gpio_free(my_dev.scl_gpio);
	free_irq(my_dev.irq_num, &my_dev);
}

module_init(stk8ba_init);
module_exit(stk8ba_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

