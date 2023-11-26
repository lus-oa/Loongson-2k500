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

#define LED_CNT 1 /* 设备号个数 */
#define LED_NAME "led" /* 名字 */
#define LEDOFF 0 /* 关led */
#define LEDON 1 /* 开led */
#define GPIO_143_136 0x1fe104d4

static void __iomem *gpio_v;

/* led设备结构体 */
struct led_dev {
	dev_t devid; /* 设备号 	 */
	struct cdev cdev; /* cdev 	*/
	struct class *class; /* 类 		*/
	struct device *device; /* 设备 	 */
	int major; /* 主设备号	  */
	int minor; /* 次设备号   */
	struct device_node *nd; /* 设备节点 */
	int green_gpio;
	int yellow_gpio;
	int red_gpio;
};

struct led_dev led; /* led设备 */

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &led; /* 设置私有数据 */
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
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt,
			 loff_t *offt)
{
	int retvalue;
	unsigned char databuf[3];
	struct led_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if (retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	if (databuf[0] == LEDON) {
		gpio_set_value(dev->green_gpio, 1);
	} else if (databuf[0] == LEDOFF) {
		gpio_set_value(dev->green_gpio, 0);
	}

	if (databuf[1] == LEDON) {
		gpio_set_value(dev->yellow_gpio, 1);
	} else if (databuf[1] == LEDOFF) {
		gpio_set_value(dev->yellow_gpio, 0);
	}

	if (databuf[2] == LEDON) {
		gpio_set_value(dev->red_gpio, 1);
	} else if (databuf[2] == LEDOFF) {
		gpio_set_value(dev->red_gpio, 0);
	}

	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.write = led_write,
	.release = led_release,
};

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init led_init(void)
{
	int ret = 0;

	led.green_gpio = 136;
	led.yellow_gpio = 138;
	led.red_gpio = 137;

	u32 gpio_read, gpio_write;
	gpio_v = ioremap(GPIO_143_136,sizeof(u32));
	gpio_read = ioread32(gpio_v);
	gpio_write = (gpio_read & 0x77777000);
	iowrite32(gpio_write, gpio_v);

	ret = gpio_request(led.green_gpio, "green_gpio");
	if (ret < 0) {
		printk("can't request green_gpio!\r\n");
	}

	ret = gpio_request(led.yellow_gpio, "yellow_gpio");
	if (ret < 0) {
		printk("can't request yellow_gpio!\r\n");
	}

	ret = gpio_request(led.red_gpio, "red_gpio");
	if (ret < 0) {
		printk("can't request red_gpio!\r\n");
	}

	ret = gpio_direction_output(led.green_gpio, 0);
	if (ret < 0) {
		printk("can't set gpio!\r\n");
	}

	ret = gpio_direction_output(led.yellow_gpio, 0);
	if (ret < 0) {
		printk("can't set gpio!\r\n");
	}

	ret = gpio_direction_output(led.red_gpio, 0);
	if (ret < 0) {
		printk("can't set gpio!\r\n");
	}

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (led.major) { /*  定义了设备号 */
		led.devid = MKDEV(led.major, 0);
		register_chrdev_region(led.devid, LED_CNT, LED_NAME);
	} else { /* 没有定义设备号 */
		alloc_chrdev_region(&led.devid, 0, LED_CNT,
				    LED_NAME); /* 申请设备号 */
		led.major = MAJOR(led.devid); /* 获取分配号的主设备号 */
		led.minor = MINOR(led.devid); /* 获取分配号的次设备号 */
	}

	/* 2、初始化cdev */
	led.cdev.owner = THIS_MODULE;
	cdev_init(&led.cdev, &led_fops);

	/* 3、添加一个cdev */
	cdev_add(&led.cdev, led.devid, LED_CNT);

	/* 4、创建类 */
	led.class = class_create(THIS_MODULE, LED_NAME);
	if (IS_ERR(led.class)) {
		return PTR_ERR(led.class);
	}

	/* 5、创建设备 */
	led.device = device_create(led.class, NULL, led.devid, NULL, LED_NAME);
	if (IS_ERR(led.device)) {
		return PTR_ERR(led.device);
	}

	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit led_exit(void)
{
	iounmap(gpio_v);
	gpio_free(led.green_gpio);
	gpio_free(led.yellow_gpio);
	gpio_free(led.red_gpio);

	/* 注销字符设备驱动 */
	cdev_del(&led.cdev); /*  删除cdev */
	unregister_chrdev_region(led.devid, LED_CNT); /* 注销设备号 */

	device_destroy(led.class, led.devid);
	class_destroy(led.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");
