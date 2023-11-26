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
#include <asm/uaccess.h>
#include <asm/io.h>

#define DAC_CNT			1
#define DAC_NAME		"dac"
#define DACOFF 			0
#define DACON 			1

#define DAC_CLOSE               10
#define DAC_OPEN                11

#define GPIO_DIN                132
#define GPIO_SCLK               133
#define GPIO_CS                 134

#define DELAY_TIME              30

#define GPCFG	0x1fe104d0

static void __iomem *GPCFG_V;

/* dac设备结构体 */
struct dac_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
	int gpio_din;			/* dac所使用的GPIO编号		*/
        int gpio_sclk;
        int gpio_cs;
};

struct dac_dev my_dac;		/* dac设备 */

static int dac_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &my_dac; /* 设置私有数据 */
	return 0;
}

static ssize_t dac_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[5];
        unsigned char cmd,i,*p,val;
        short tmp;
	struct dac_dev *dev = filp->private_data;

	retvalue = copy_from_user(databuf, buf, cnt);
	if(retvalue < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

        p = &databuf[1];
        memcpy((void*)&tmp,(void*)p,2);
        //printk("dac_write====>tmp:%x\r\n",tmp);

	cmd = databuf[0];
        if(DAC_CLOSE==cmd) {
           gpio_set_value(dev->gpio_din, 0);
           gpio_set_value(dev->gpio_sclk, 0);
           gpio_set_value(dev->gpio_cs, 1);
        } else if(DAC_OPEN==cmd) {
           //printk("dac_write======>open\r\n");
           gpio_set_value(dev->gpio_sclk, 0);
           gpio_set_value(dev->gpio_cs, 0);
           ndelay(DELAY_TIME/2);
           gpio_set_value(dev->gpio_sclk, 1);
           gpio_set_value(dev->gpio_cs, 0);
           ndelay(DELAY_TIME/2);
           gpio_set_value(dev->gpio_sclk, 0);
           gpio_set_value(dev->gpio_cs, 1);
           ndelay(DELAY_TIME/2);
           gpio_set_value(dev->gpio_sclk, 1);
           ndelay(DELAY_TIME/2);
           gpio_set_value(dev->gpio_cs, 0);

           for(i=0;i<16;i++) {           
             gpio_set_value(dev->gpio_sclk, 0);
             val = ((tmp & 0x8000) ? 1:0);
             //gpio_set_value(dev->gpio_din, databuf[i+1]);
             gpio_set_value(dev->gpio_din, val);
             tmp <<=1;
             ndelay(DELAY_TIME/2);
             gpio_set_value(dev->gpio_sclk, 1);
             ndelay(DELAY_TIME/2);
           }
           gpio_set_value(dev->gpio_sclk, 0);
           gpio_set_value(dev->gpio_cs, 1);
        }

        //printk("dac_write: buf[0]:%d,buf[1]:%d,buf[2]:%d,buf[3]:%d,buf[4]:%d,buf[5]:%d\r\n",databuf[0],databuf[1],databuf[2],databuf[3],databuf[4],databuf[5]);	
	return 0;
}

static int dac_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations dac_fops = {
	.owner = THIS_MODULE,
	.open = dac_open,
	.write = dac_write,
	.release = dac_release,
};


/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init dac_init(void)
{
	int ret = 0;
	u32 GPCFG_read, GPCFG_write;

	GPCFG_V = ioremap(GPCFG, sizeof(u32));
	GPCFG_read = ioread32(GPCFG_V);
	GPCFG_write = (GPCFG_read & 0x70007777);
	iowrite32(GPCFG_write, GPCFG_V);

	my_dac.gpio_din = GPIO_DIN;
        my_dac.gpio_sclk = GPIO_SCLK;
        my_dac.gpio_cs = GPIO_CS;

        gpio_request(my_dac.gpio_din, "din");
	ret = gpio_direction_output(my_dac.gpio_din, 1);
	if(ret < 0) {
		printk("can't set din gpio!\r\n");
	}
   
        gpio_request(my_dac.gpio_sclk, "sclk");
        ret = gpio_direction_output(my_dac.gpio_sclk, 1);
	if(ret < 0) {
		printk("can't set sclk gpio!\r\n");
	}

        gpio_request(my_dac.gpio_cs, "cs");
        ret = gpio_direction_output(my_dac.gpio_cs, 1);
	if(ret < 0) {
		printk("can't set cs gpio!\r\n");
	}

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (my_dac.major) {		/*  定义了设备号 */
		my_dac.devid = MKDEV(my_dac.major, 0);
		register_chrdev_region(my_dac.devid, DAC_CNT, DAC_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&my_dac.devid, 0, DAC_CNT, DAC_NAME);	/* 申请设备号 */
		my_dac.major = MAJOR(my_dac.devid);	/* 获取分配号的主设备号 */
		my_dac.minor = MINOR(my_dac.devid);	/* 获取分配号的次设备号 */
	}
	printk("dac major=%d,minor=%d\r\n",my_dac.major, my_dac.minor);	
	
	/* 2、初始化cdev */
	my_dac.cdev.owner = THIS_MODULE;
	cdev_init(&my_dac.cdev, &dac_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&my_dac.cdev, my_dac.devid, DAC_CNT);

	/* 4、创建类 */
	my_dac.class = class_create(THIS_MODULE, DAC_NAME);
	if (IS_ERR(my_dac.class)) {
		return PTR_ERR(my_dac.class);
	}

	/* 5、创建设备 */
	my_dac.device = device_create(my_dac.class, NULL, my_dac.devid, NULL, DAC_NAME);
	if (IS_ERR(my_dac.device)) {
		return PTR_ERR(my_dac.device);
	}
	
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit dac_exit(void)
{
	/* 注销字符设备驱动 */
	iounmap(GPCFG_V);
	cdev_del(&my_dac.cdev);/*  删除cdev */
	unregister_chrdev_region(my_dac.devid, DAC_CNT); /* 注销设备号 */

	device_destroy(my_dac.class, my_dac.devid);
	class_destroy(my_dac.class);
}

module_init(dac_init);
module_exit(dac_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");



