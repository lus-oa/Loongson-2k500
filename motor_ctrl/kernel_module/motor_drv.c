// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

//gpio84===pwm0,gpio85===pwm1
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

#define MOTOR_NAME		"motor"	/* 名字 	*/
#define MOTOR_MINOR		150			/* 子设备号 */

#define  MAGIC_NUMBER    'k'
#define CMD_standby _IO(MAGIC_NUMBER    ,1)
#define CMD_forward  _IO(MAGIC_NUMBER    ,2)
#define CMD_backward  _IO(MAGIC_NUMBER    ,3)
#define CMD_brake _IO(MAGIC_NUMBER    ,4)

#define   INA_GPIO   84
#define   INB_GPIO   85

#define DELAY_TIME      10

//GPIO 87-80
#define GPCFG 0x1fe104b8
static void __iomem *GPCFG_V;

struct motor_dev{
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int ina_gpio;
        int inb_gpio;
};

struct motor_dev misc_motor;

static void gpio_set(int ina,int inb)
{
       gpio_set_value(misc_motor.ina_gpio, ina);
       udelay(DELAY_TIME);
       gpio_set_value(misc_motor.inb_gpio, inb);
}

static int motor_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &misc_motor; /* 设置私有数据 */
	return 0;
}

static long motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
       switch(cmd) {
           case CMD_standby:
                gpio_set(0,0);
                break;
           case CMD_forward:
                gpio_set(1,0);
                break;
           case CMD_backward:
                gpio_set(0,1);
                break;
           case CMD_brake:
                gpio_set(1,1);
                break;
           default:
                break;
       };
 
       return 0;
}

/* 设备操作函数 */
static struct file_operations misc_motor_fops = {
	.owner = THIS_MODULE,
	.open = motor_open,
    .unlocked_ioctl = motor_ioctl,
};

/* MISC设备结构体 */
static struct miscdevice motor_miscdev = {
	.minor = MOTOR_MINOR,
	.name = MOTOR_NAME,
	.fops = &misc_motor_fops,
};

static int __init motor_init(void)
{
	int ret;
	int i;
	u32 GPCFG_read, GPCFG_write;

	ret = misc_register(&motor_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

	GPCFG_V = ioremap(GPCFG, sizeof(u32));
	GPCFG_read = ioread32(GPCFG_V);
	GPCFG_write = (GPCFG_read & 0x77007777);
	iowrite32(GPCFG_write, GPCFG_V);

	misc_motor.ina_gpio = INA_GPIO;
	misc_motor.inb_gpio = INB_GPIO;

	gpio_request(misc_motor.ina_gpio, "ina");
	gpio_request(misc_motor.inb_gpio, "inb");

	ret = gpio_direction_output(misc_motor.ina_gpio, 0);
	if(ret < 0) {
		printk("can't set ina gpio!\r\n");
	}    
 
	ret = gpio_direction_output(misc_motor.inb_gpio, 0);
	if(ret < 0) {
		printk("can't set inb gpio!\r\n");
	}

	return 0;
}

static void __exit motor_exit(void)
{
	misc_deregister(&motor_miscdev);

	iounmap(GPCFG_V);

	gpio_free(misc_motor.ina_gpio);
	gpio_free(misc_motor.inb_gpio);
}

module_init(motor_init);
module_exit(motor_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");

