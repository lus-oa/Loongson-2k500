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
#include "softi2c.h"

#define STK8BA_NAME		"stk8ba"	/* 名字 	*/
#define STK8BA_MINOR		148			/* 子设备号 */
#define STK8BA_ADDR		0x18
#define STK8BA_INT_GPIO        100

#define GPCFG	0x1fe104c0	//INT管脚复用配置寄存器
/* __iomem  虚拟地址指针，保存虚拟地址*/
static void __iomem *GPCFG_V;

struct stk8ba_struct {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        I2c_Pins pins;
	int irq_num;
	unsigned char write_addr;
	unsigned char read_addr;
};

struct stk8ba_struct stk8ba_dev;

void STK8BA_Write_Reg(struct stk8ba_struct *dev, unsigned char reg_addr, unsigned char new_value)
{
	get_i2c_lock(&dev->pins);

   	I2c_Start(&dev->pins);    // 启动总线
	I2c_WrByte(&dev->pins, dev->write_addr);
	I2c_WrByte(&dev->pins, reg_addr);
	I2c_WrByte(&dev->pins, new_value);
	I2c_Stop(&dev->pins);    // 结束总线  

	free_i2c_lock(&dev->pins);
}

static unsigned char STK8BA_Read_Reg(struct stk8ba_struct *dev, unsigned char reg)
{
	unsigned char val;

	get_i2c_lock(&dev->pins);

	I2c_Start(&dev->pins);
	I2c_WrByte(&dev->pins, dev->write_addr);
	I2c_WrByte(&dev->pins, reg);
	I2c_Stop(&dev->pins);
	
	I2c_Start(&dev->pins);
	I2c_WrByte(&dev->pins, dev->read_addr);
	val = I2c_RdByte(&dev->pins, NACK);
	I2c_Stop(&dev->pins);

	free_i2c_lock(&dev->pins);
	
	return val;
}

static void stk8ba_read_xyz(struct stk8ba_struct *dev, unsigned char *data)
{
	data[0] = STK8BA_Read_Reg(dev, STK8BA_XOUT1);
        data[1] = STK8BA_Read_Reg(dev, STK8BA_XOUT2);
        data[2] = STK8BA_Read_Reg(dev, STK8BA_YOUT1);
        data[3] = STK8BA_Read_Reg(dev, STK8BA_YOUT2);
        data[4] = STK8BA_Read_Reg(dev, STK8BA_ZOUT1);
        data[5] = STK8BA_Read_Reg(dev, STK8BA_ZOUT2);
}

static int stk8ba_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &stk8ba_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t stk8ba_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	unsigned char data[6];
	int ret;

	struct stk8ba_struct *dev = filp->private_data;
	
	stk8ba_read_xyz(dev, data);

	ret = copy_to_user(buf, data, cnt);
	if (ret < 0)
	{
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

        return cnt;
}


static int stk8ba_release(struct inode *inode, struct file *filp)
{
	return 0;
}

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
	.release = stk8ba_release,
};

/* MISC设备结构体 */
static struct miscdevice stk8ba_miscdev = {
	.minor = STK8BA_MINOR,
	.name = STK8BA_NAME,
	.fops = &stk8ba_ops,
};


static int __init stk8ba_init(void)
{
        int ret;
	u32 GPCFG_read, GPCFG_write;

	GPCFG_V = ioremap(GPCFG, sizeof(u32));
	GPCFG_read = ioread32(GPCFG_V);
	GPCFG_write = (GPCFG_read & 0x77707777);
	iowrite32(GPCFG_write, GPCFG_V);
	iounmap(GPCFG_V);

	gpio_request(STK8BA_INT_GPIO, "stk8ba_int");
	
	stk8ba_dev.pins.sda = SDA3_GPIO;
        stk8ba_dev.pins.scl = SCL3_GPIO;
	stk8ba_dev.irq_num = gpio_to_irq(STK8BA_INT_GPIO);
	stk8ba_dev.write_addr = I2C_WRITE_ADDR(STK8BA_ADDR);
	stk8ba_dev.read_addr = I2C_READ_ADDR(STK8BA_ADDR);
	pr_info("stk8ba_init gpio int irq num:%d\n", stk8ba_dev.irq_num);

	STK8BA_Write_Reg(&stk8ba_dev, STK8BA_INTMAP1, 0x5);
        STK8BA_Write_Reg(&stk8ba_dev, STK8BA_INTMAP2, 0x1);

        STK8BA_Write_Reg(&stk8ba_dev, STK8BA_INTEN1, 0x7);
        STK8BA_Write_Reg(&stk8ba_dev, STK8BA_INTEN2, 0x10);

	ret = request_irq(stk8ba_dev.irq_num, read_handler, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "stk8ba-irq", &stk8ba_dev);

	pr_info("intcfg1:%x, intcfg2:%x\n", STK8BA_Read_Reg(&stk8ba_dev, STK8BA_INTCFG1), STK8BA_Read_Reg(&stk8ba_dev, STK8BA_INTCFG2));

	ret = misc_register(&stk8ba_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}
  
        return ret;
}

static void __exit stk8ba_exit(void)
{
       	misc_deregister(&stk8ba_miscdev);
	free_irq(stk8ba_dev.irq_num, &stk8ba_dev);
	gpio_free(STK8BA_INT_GPIO);
}

module_init(stk8ba_init);
module_exit(stk8ba_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

