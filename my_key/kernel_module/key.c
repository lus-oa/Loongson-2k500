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
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define KEY_CNT			1		/* 设备号个数 	*/
#define KEY_NAME		"key"	/* 名字 		*/

/* 定义按键值 */
#define KEY0VALUE		0XF0	/* 按键值 		*/
#define INVAKEY			0X00	/* 无效的按键值  */

#define GPIO_96_103 0x1fe104c0
#define GPIO_104_111 0x1fe104c4

static void __iomem *gpio_103_96_v;
static void __iomem *gpio_111_104_v;

/* key设备结构体 */
struct key_dev{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;	/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
	struct device_node	*nd; /* 设备节点 */
	int key_gpio[8];			/* key所使用的GPIO编号		*/
	atomic_t keyvalue;		/* 按键值 		*/	
};

struct key_dev keydev;		/* key设备 */
static int g_flag = 0;

/*
 * @description	: 初始化按键IO，open函数打开驱动的时候
 * 				  初始化按键所使用的GPIO引脚。
 * @param 		: 无
 * @return 		: 无
 */
static int keyio_init(void)
{
        int ret;
	u32 gpio_read,gpio_write;
    gpio_103_96_v = ioremap(GPIO_96_103,sizeof(u32));
    gpio_read = ioread32(gpio_103_96_v);
    gpio_write = (gpio_read & 0x07777777);
    iowrite32(gpio_write,gpio_103_96_v);

	gpio_111_104_v = ioremap(GPIO_104_111,sizeof(u32));
    gpio_read = ioread32(gpio_111_104_v);
    gpio_write = (gpio_read & 0x70000000);
    iowrite32(gpio_write,gpio_111_104_v);

	keydev.key_gpio[0] = 103;
        keydev.key_gpio[1] = 104;
        keydev.key_gpio[2] = 105;
        keydev.key_gpio[3] = 106;
        keydev.key_gpio[4] = 107;
        keydev.key_gpio[5] = 108;
        keydev.key_gpio[6] = 109;
        keydev.key_gpio[7] = 110;
	//printk("key_gpio[0]=%d\r\n", keydev.key_gpio[0]);
	
        if(g_flag)
           goto L1;
   
	/* 初始化key所使用的IO */
	ret = gpio_request(keydev.key_gpio[0], "key0");	/* 请求IO */
	gpio_direction_input(keydev.key_gpio[0]);	/* 设置为输入 */
        //printk("key_gpio[0] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[1], "key1");	/* 请求IO */
	gpio_direction_input(keydev.key_gpio[1]);	/* 设置为输入 */
        //printk("key_gpio[1] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[2], "key2");	/* 请求IO */
	gpio_direction_input(keydev.key_gpio[2]);	/* 设置为输入 */
        //printk("key_gpio[2] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[3], "key3");	/* 请求IO */
	gpio_direction_input(keydev.key_gpio[3]);	/* 设置为输入 */
        //printk("key_gpio[0] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[4], "key4");	/* 请求IO */
	gpio_direction_output(keydev.key_gpio[4], 0); /* 设置为输出*/
        //printk("key_gpio[4] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[5], "key5");	/* 请求IO */
	gpio_direction_output(keydev.key_gpio[5], 0); /* 设置为输出*/
        //printk("key_gpio[5] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[6], "key6");	/* 请求IO */
	gpio_direction_output(keydev.key_gpio[6], 0); /* 设置为输出*/
        //printk("key_gpio[6] ret:%d\r\n",ret);

        ret = gpio_request(keydev.key_gpio[7], "key7");	/* 请求IO */
	gpio_direction_output(keydev.key_gpio[7], 0); /* 设置为输出*/
        //printk("key_gpio[7] ret:%d\r\n",ret);

        g_flag = 1;
L1:
	return 0;
}

/*
 * @description		: 打开设备
 * @param - inode 	: 传递给驱动的inode
 * @param - filp 	: 设备文件，file结构体有个叫做private_data的成员变量
 * 					  一般在open的时候将private_data指向设备结构体。
 * @return 			: 0 成功;其他 失败
 */
static int key_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	filp->private_data = &keydev; 	/* 设置私有数据 */

	ret = keyio_init();				/* 初始化按键IO */
	if (ret < 0) {
		return ret;
	}

	return 0;
}

/*
 * @description		: 从设备读取数据 
 * @param - filp 	: 要打开的设备文件(文件描述符)
 * @param - buf 	: 返回给用户空间的数据缓冲区
 * @param - cnt 	: 要读取的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 读取的字节数，如果为负值，表示读取失败
 */
static ssize_t key_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int ret = 0;
	int keyvalue[8];
        int i;
	struct key_dev *dev = filp->private_data;

        for(i=0;i<8;i++)
        {
             keyvalue[i] = gpio_get_value(dev->key_gpio[i]);
        }
	
	ret = copy_to_user(buf, keyvalue, sizeof(keyvalue));
	return ret;
}

/*
 * @description		: 向设备写数据 
 * @param - filp 	: 设备文件，表示打开的文件描述符
 * @param - buf 	: 要写给设备写入的数据
 * @param - cnt 	: 要写入的数据长度
 * @param - offt 	: 相对于文件首地址的偏移
 * @return 			: 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t key_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
        int ret;
	unsigned char databuf[4];
        struct key_dev *dev = filp->private_data;

	ret = copy_from_user(databuf, buf, cnt);
	if(ret < 0) {
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

        //printk("key_gpio[4]:%d,key_gpio[5]:%d,key_gpio[6]:%d,key_gpio[7]:%d\r\n",databuf[0],databuf[1],databuf[2],databuf[3]);
        gpio_set_value(dev->key_gpio[4], databuf[0]);
        gpio_set_value(dev->key_gpio[5], databuf[1]);
        gpio_set_value(dev->key_gpio[6], databuf[2]);
        gpio_set_value(dev->key_gpio[7], databuf[3]);

	return 0;
}

/*
 * @description		: 关闭/释放设备
 * @param - filp 	: 要关闭的设备文件(文件描述符)
 * @return 			: 0 成功;其他 失败
 */
static int key_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* 设备操作函数 */
static struct file_operations key_fops = {
	.owner = THIS_MODULE,
	.open = key_open,
	.read = key_read,
	.write = key_write,
	.release = 	key_release,
};

/*
 * @description	: 驱动入口函数
 * @param 		: 无
 * @return 		: 无
 */
static int __init mykey_init(void)
{
	/* 初始化原子变量 */
	atomic_set(&keydev.keyvalue, INVAKEY);

	/* 注册字符设备驱动 */
	/* 1、创建设备号 */
	if (keydev.major) {		/*  定义了设备号 */
		keydev.devid = MKDEV(keydev.major, 0);
		register_chrdev_region(keydev.devid, KEY_CNT, KEY_NAME);
	} else {						/* 没有定义设备号 */
		alloc_chrdev_region(&keydev.devid, 0, KEY_CNT, KEY_NAME);	/* 申请设备号 */
		keydev.major = MAJOR(keydev.devid);	/* 获取分配号的主设备号 */
		keydev.minor = MINOR(keydev.devid);	/* 获取分配号的次设备号 */
	}
	
	/* 2、初始化cdev */
	keydev.cdev.owner = THIS_MODULE;
	cdev_init(&keydev.cdev, &key_fops);
	
	/* 3、添加一个cdev */
	cdev_add(&keydev.cdev, keydev.devid, KEY_CNT);

	/* 4、创建类 */
	keydev.class = class_create(THIS_MODULE, KEY_NAME);
	if (IS_ERR(keydev.class)) {
		return PTR_ERR(keydev.class);
	}

	/* 5、创建设备 */
	keydev.device = device_create(keydev.class, NULL, keydev.devid, NULL, KEY_NAME);
	if (IS_ERR(keydev.device)) {
		return PTR_ERR(keydev.device);
	}
	
	return 0;
}

/*
 * @description	: 驱动出口函数
 * @param 		: 无
 * @return 		: 无
 */
static void __exit mykey_exit(void)
{
        int i;
	iounmap(gpio_111_104_v);
	iounmap(gpio_103_96_v);
	/* 注销字符设备驱动 */
        for(i=0;i<8;i++)
        {
        	gpio_free(keydev.key_gpio[i]);
        }
	cdev_del(&keydev.cdev);/*  删除cdev */
	unregister_chrdev_region(keydev.devid, KEY_CNT); /* 注销设备号 */

	device_destroy(keydev.class, keydev.devid);
	class_destroy(keydev.class);
}

module_init(mykey_init);
module_exit(mykey_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");
