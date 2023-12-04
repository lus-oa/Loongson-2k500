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
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define BEEP_CNT 1 /* 设备号个数 */
#define BEEP_NAME "beep" /* 名字 */
#define BEEPOFF 0 /* 关蜂鸣器 */
#define BEEPON 1 /* 开蜂鸣器 */
#define GPMUX 0x1fe104a0
#define u32 __u32

static void __iomem *GPFMUX0_V;

/* beep设备结构体 */
struct beep_dev {
	dev_t devid; /* 设备号 	 */
	struct cdev cdev; /* cdev 	*/
	struct class *class; /* 类 		*/
	struct device *device; /* 设备 	 */
	int major; /* 主设备号	  */
	int minor; /* 次设备号   */
	struct device_node *nd; /* 设备节点 */
	int beep_gpio; /* beep所使用的GPIO编号		*/
};

struct beep_dev beep; /* beep设备 */

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int beep_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &beep; /* 设置私有数据 */
	return 0;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt,
			  loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char beepstat;
	struct beep_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if (retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	beepstat = databuf[0]; /* 获取状态值 */

	if (beepstat == BEEPON) {
		gpio_set_value(dev->beep_gpio, 0); /* 打开蜂鸣器 */
	} else if (beepstat == BEEPOFF) {
		gpio_set_value(dev->beep_gpio, 1); /* 关闭蜂鸣器 */
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int beep_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations beep_fops = {
	.owner = THIS_MODULE,
	.open = beep_open,
	.write = beep_write,
	.release = beep_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init beep_init(void)
{
	int ret = 0;
	u32 GPFMUX_READ, GPFMUX_WRITE;
	GPFMUX0_V = ioremap(GPMUX, sizeof(u32));
	GPFMUX_READ = ioread32(GPFMUX0_V);
	GPFMUX_WRITE = (GPFMUX_READ & 0x77777707);
	iowrite32(GPFMUX_WRITE,GPFMUX0_V);


	beep.beep_gpio = 33;
	gpio_request(beep.beep_gpio, "sda3");
	/* 设置GPIO5_IO01为输出，并且输出高电平，默认关闭BEEP */
	ret = gpio_direction_output(beep.beep_gpio, 1);
	if (ret < 0) {
		printk("can't set gpio!\r\n");
	}

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (beep.major) { /*  定义了设备号 */
		beep.devid = MKDEV(beep.major, 0);
		register_chrdev_region(beep.devid, BEEP_CNT, BEEP_NAME);
	} else { /* 没有定义设备号 */
		alloc_chrdev_region(&beep.devid, 0, BEEP_CNT,
				    BEEP_NAME); /* 申请设备号 */
		beep.major = MAJOR(beep.devid); /* 获取分配号的主设备号 */
		beep.minor = MINOR(beep.devid); /* 获取分配号的次设备号 */
	}
	printk("beep major=%d,minor=%d\r\n", beep.major, beep.minor);

	/* 2、初始化cdev */
	beep.cdev.owner = THIS_MODULE;
	cdev_init(&beep.cdev, &beep_fops);

	/* 3、添加一个cdev */
	cdev_add(&beep.cdev, beep.devid, BEEP_CNT);

	/* 4、创建类 */
	beep.class = class_create(THIS_MODULE, BEEP_NAME);
	if (IS_ERR(beep.class)) {
		return PTR_ERR(beep.class);
	}

	/* 5、创建设备 */
	beep.device =
		device_create(beep.class, NULL, beep.devid, NULL, BEEP_NAME);
	if (IS_ERR(beep.device)) {
		return PTR_ERR(beep.device);
	}

	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit beep_exit(void)
{
	iounmap(GPFMUX0_V);
	/* 注销字符设备驱动 */
	cdev_del(&beep.cdev); /*  删除cdev */
	unregister_chrdev_region(beep.devid, BEEP_CNT); /* 注销设备号 */

	device_destroy(beep.class, beep.devid);
	class_destroy(beep.class);
}

module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");
