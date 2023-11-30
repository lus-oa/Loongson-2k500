#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "softi2c.h"

#define AHT20_NAME	"aht20"		/* 设备名  */
#define AHT20_MINOR	147		/* 子设备号 */
#define AHT20_ADDR	0x38

struct aht20_struct {
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        I2c_Pins pins;
	unsigned char write_addr;
	unsigned char read_addr;
};

static struct aht20_struct aht20_dev;

void AHT20_SendCmd( struct aht20_struct *dev, unsigned char *cmd_buf, size_t cnt )
{
	int i;
	get_i2c_lock(&dev->pins);
   	I2c_Start(&dev->pins);    // 启动总线
	I2c_WrByte(&dev->pins, dev->write_addr);
	for (i = 0; i < cnt; i++)
	{
		I2c_WrByte(&dev->pins, (unsigned char)(cmd_buf[i]));
	} 
	I2c_Stop(&dev->pins);    // 结束总线  
	free_i2c_lock(&dev->pins);
}

void AHT20_Recv( struct aht20_struct *dev, unsigned char *buf, size_t cnt )
{
	int i;
	get_i2c_lock(&dev->pins);
	I2c_Start(&dev->pins);
	I2c_WrByte(&dev->pins, dev->read_addr);
	for (i = 0; i < cnt; i++)
	{
		buf[i] = I2c_RdByte(&dev->pins, cnt - i - 1);
	}
	I2c_Stop(&dev->pins);
	free_i2c_lock(&dev->pins);
}

static int aht20_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &aht20_dev; /* 设置私有数据 */
	return 0;
}

static ssize_t aht20_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	unsigned char databuf[7], cmd[3];
	int ret;
	struct aht20_struct *dev = (struct aht20_struct *)filp->private_data;
	
	mdelay(50);

	cmd[0] = 0xac;
	cmd[1] = 0x33;
	cmd[2] = 0x00;
	AHT20_SendCmd(dev, cmd, 3);

	mdelay(80);

	AHT20_Recv(dev, databuf, 7);

	ret = copy_to_user(buf, databuf, cnt);
	if (ret < 0)
	{
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

        return cnt;
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
	.minor = AHT20_MINOR,
	.name = AHT20_NAME,
	.fops = &aht20_ops,
};

static int __init aht20_init(void)
{
        int ret = 0;

	printk("aht20 module init\n");
        
        aht20_dev.pins.sda = SDA3_GPIO;
        aht20_dev.pins.scl = SCL3_GPIO;
	aht20_dev.write_addr = I2C_WRITE_ADDR(AHT20_ADDR);
	aht20_dev.read_addr = I2C_READ_ADDR(AHT20_ADDR);

	ret = misc_register(&aht20_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

	return ret;
}

static void __exit aht20_exit(void)
{
	printk("aht20 module exit\n");
       	misc_deregister(&aht20_miscdev);
}

module_init(aht20_init);
module_exit(aht20_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yangjinrun");

