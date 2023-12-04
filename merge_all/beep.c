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
#include <asm/io.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>

#define BEEP_CNT	1		/* 设备号个数 */
#define BEEP_NAME	"beep"		/* 名字 */
#define BEEP_GPIO	33

#define BEEP_MAGIC	'b'
#define BEEP_OFF 	_IO(BEEP_MAGIC, 1)	/* 关蜂鸣器 */
#define BEEP_ON 	_IO(BEEP_MAGIC, 2)		/* 开蜂鸣器 */

//GPIO32~39的复用配置寄存器
#define GPCFG 0x1fe104a0
static void __iomem *GPCFG_V;  

/* beep设备结构体 */
struct beep_struct {
	dev_t devid;			/* 设备号 */
	struct cdev cdev;		/* cdev */
	struct class *class;		/* 类 	*/
	struct device *device;		/* 设备  */
	int major;			/* 主设备号  */
	int minor;			/* 次设备号  */
	struct device_node	*nd; 	/* 设备节点 */
	int beep_gpio;			/* beep所使用的GPIO编号	*/
};

struct beep_struct beep_dev;		/* beep设备 */

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 			  一般在open的时候将private_data指向设备结构体。
 * @return 		: 0 成功;其他 失败
 */
static int beep_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &beep_dev; /* 设置私有数据 */
	return 0;
}

static long beep_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct beep_struct *dev = filp->private_data;

	if (_IOC_TYPE(cmd) != BEEP_MAGIC) 
		return -EINVAL;

	switch (cmd)
	{
	case BEEP_ON:
		gpio_set_value(dev->beep_gpio, 0);	/* 打开蜂鸣器 */
		break;
	case BEEP_OFF:
		gpio_set_value(dev->beep_gpio, 1);	/* 关闭蜂鸣器 */
		break;
	default:
		break;
	}
	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 		: 0 成功;其他 失败
 */
static int beep_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations beep_fops = {
	.owner = THIS_MODULE,
	.open = beep_open,
	.unlocked_ioctl = beep_ioctl,
	.release = beep_release,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init beep_init(void)
{
	int ret = 0;
	u32 GPCFG_write, GPCFG_read;

	//将GPIO33引脚复用配置为GPIO（000）
	GPCFG_V = ioremap(GPCFG, sizeof(u32));
	GPCFG_read = ioread32(GPCFG_V);
	GPCFG_write = (GPCFG_read & 0x77777707);
	iowrite32(GPCFG_write, GPCFG_V);
	iounmap(GPCFG_V);

	beep_dev.beep_gpio = BEEP_GPIO;
	/* 设置GPIO5_IO01为输出，并且输出高电平，默认关闭BEEP */
	gpio_request(beep_dev.beep_gpio, "beep_io");
	ret = gpio_direction_output(beep_dev.beep_gpio, 1);
	if(ret < 0) {
		printk("can't set gpio!\r\n");
	}

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (beep_dev.major) {		/*  定义了设备号 */
		beep_dev.devid = MKDEV(beep_dev.major, 0);
		register_chrdev_region(beep_dev.devid, BEEP_CNT, BEEP_NAME);
	} else {			/* 没有定义设备号 */
		alloc_chrdev_region(&beep_dev.devid, 0, BEEP_CNT, BEEP_NAME);	/* 申请设备号 */
		beep_dev.major = MAJOR(beep_dev.devid);	/* 获取分配号的主设备号 */
		beep_dev.minor = MINOR(beep_dev.devid);	/* 获取分配号的次设备号 */
	}
	printk("beep major=%d,minor=%d\r\n",beep_dev.major, beep_dev.minor);	
	
	/* 2、初始化cdev */
	beep_dev.cdev.owner = THIS_MODULE;
	cdev_init(&beep_dev.cdev, &beep_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&beep_dev.cdev, beep_dev.devid, BEEP_CNT);

	/* 4、创建类 */
	beep_dev.class = class_create(THIS_MODULE, BEEP_NAME);
	if (IS_ERR(beep_dev.class)) {
		return PTR_ERR(beep_dev.class);
	}

	/* 5、创建设备 */
	beep_dev.device = device_create(beep_dev.class, NULL, beep_dev.devid, NULL, BEEP_NAME);
	if (IS_ERR(beep_dev.device)) {
		return PTR_ERR(beep_dev.device);
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
	/* 注销字符设备驱动 */
	cdev_del(&beep_dev.cdev);/*  删除cdev */
	unregister_chrdev_region(beep_dev.devid, BEEP_CNT); /* 注销设备号 */

	device_destroy(beep_dev.class, beep_dev.devid);
	class_destroy(beep_dev.class);
}

module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");
