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
#include "ch422g.h"
#include "softi2c.h"

#define CH422G_NAME		"ch422g"	/* 名字 	*/
#define CH422G_MINOR		146			/* 子设备号 */

struct ch422g_struct {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        I2c_Pins pins;
};

struct ch422g_struct ch422g_dev;

static void CH422_SendCmd(struct ch422g_struct *dev, unsigned short cmd)
{
	//因为ch422g使用单独的GPIO，可以不用加锁
	//get_i2c_lock(&dev->pins);
	I2c_Start(&dev->pins);    // 启动总线
	I2c_WrByte(&dev->pins, (unsigned char)(cmd>>8));
	I2c_WrByte(&dev->pins, (unsigned char)cmd);    // 发送数据
	I2c_Stop(&dev->pins);    // 结束总线
	//free_i2c_lock(&dev->pins);
}

static int ch422g_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &ch422g_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t ch422g_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[5];
	struct ch422g_struct *dev = filp->private_data;
        unsigned char cmd,ch1,ch2,ch3,ch4;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}
        cmd = databuf[0];
        ch1 = databuf[1];
        ch2 = databuf[2];
        ch3 = databuf[3];
        ch4 = databuf[4];

        CH422_SendCmd(dev, CH422_SET_IO_CMD + (0x07)); //0111
        CH422_SendCmd(dev, CH422_DIG0 | ch1);

        CH422_SendCmd(dev, CH422_SET_IO_CMD + (0x0b)); //1011
        CH422_SendCmd(dev, CH422_DIG1 | ch2);

        CH422_SendCmd(dev, CH422_SET_IO_CMD + (0x0d)); //1101
        CH422_SendCmd(dev, CH422_DIG2 | ch3);

        CH422_SendCmd(dev, CH422_SET_IO_CMD + (0x0e)); //1110
        CH422_SendCmd(dev, CH422_DIG3 | ch4);

        return 0;
}


/* 设备操作函数 */
static struct file_operations misc_ch422g_fops = {
	.owner = THIS_MODULE,
	.open = ch422g_open,
        .write = ch422g_write,
};

/* MISC设备结构体 */
static struct miscdevice ch422g_miscdev = {
	.minor = CH422G_MINOR,
	.name = CH422G_NAME,
	.fops = &misc_ch422g_fops,
};


static int __init ch422g_init(void)
{
        int ret;

	printk("ch422g module init\n");

        ch422g_dev.pins.sda = CH422G_SDA_GPIO;
        ch422g_dev.pins.scl = CH422G_SCL_GPIO;

	ret = misc_register(&ch422g_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

        CH422_SendCmd(&ch422g_dev, CH422_SYS_CMD | CH422_IO_OE_BIT | CH422_A_SCAN_BIT);
        
        return ret;
}

static void __exit ch422g_exit(void)
{
	printk("ch422g module exit\n");
       	misc_deregister(&ch422g_miscdev);
}

module_init(ch422g_init);
module_exit(ch422g_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

