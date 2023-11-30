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
#include "aip1944.h"

#define  MAGIC_NUMBER    'k'
#define CMD_qian _IO(MAGIC_NUMBER    ,1)
#define CMD_wei  _IO(MAGIC_NUMBER    ,2)
#define CMD_zhi  _IO(MAGIC_NUMBER    ,3)
#define CMD_neng _IO(MAGIC_NUMBER    ,4)
#define CMD_ke   _IO(MAGIC_NUMBER    ,5)
#define CMD_ji   _IO(MAGIC_NUMBER    ,6)

#define AIP_NAME		"aip"	/* 名字 	*/
#define AIP_MINOR		166			/* 子设备号 */
#define DELAY_TIME      10
#define GPIO5548ADDR 0x1fe104a8
#define GPIO119112  0x1fe104c8


static void __iomem *GPIO5548ADDR_V;
static void __iomem *GPIO119112_V;


struct aip_dev{
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int clk_gpio;
        int dio_gpio;
        int stb_gpio;
};

struct aip_dev misc_aip;
static void display_arr(unsigned char *dat);


static int aip_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &misc_aip; /* 设置私有数据 */
	return 0;
}

static ssize_t aip_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
        int ret;
	unsigned char databuf[64];
	struct aip_dev *dev = filp->private_data;

        ret = copy_from_user(databuf, buf, cnt);
	if(ret < 0) {
		pr_info("kernel write failed!\r\n");
		return -EFAULT;
	}

        display_arr(databuf);
        return cnt;
}

// static long aip_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
// {
//        switch(cmd) {
//            case CMD_qian:
//                 display_arr(shan);
//                 break;
//            case CMD_wei:
//                 display_arr(wei);
//                 break;
//            case CMD_zhi:
//                 display_arr(zhi);
//                 break;
//            case CMD_neng:
//                 display_arr(neng);
//                 break;
//            case CMD_ke:
//                 display_arr(ke);
//                 break;
//            case CMD_ji:
//                 display_arr(ji);
//                 break;
//            default:
//                 break;
//        };
 
//        return 0;
// }

/* 设备操作函数 */
static struct file_operations misc_aip_fops = {
	.owner = THIS_MODULE,
	.open = aip_open,
        .write = aip_write,
//     .unlocked_ioctl = aip_ioctl,
};

/* MISC设备结构体 */
static struct miscdevice aip_miscdev = {
	.minor = AIP_MINOR,
	.name = AIP_NAME,
	.fops = &misc_aip_fops,
};

static void write(unsigned char value)
{
  unsigned char i,dio;
 
  gpio_set_value(misc_aip.stb_gpio, 0);
  for(i=0;i<8;i++){
    gpio_set_value(misc_aip.clk_gpio, 0);
    udelay(DELAY_TIME);
    dio = value & 0x01;
    gpio_set_value(misc_aip.dio_gpio, dio);
    udelay(DELAY_TIME);
    gpio_set_value(misc_aip.clk_gpio, 1);
    udelay(DELAY_TIME);
    value >>= 1;
  }
  udelay(DELAY_TIME);
}

static void write_cmd(unsigned char cmd)
{
        gpio_set_value(misc_aip.stb_gpio, 1);
	udelay(DELAY_TIME);
	gpio_set_value(misc_aip.stb_gpio, 0);
	write(cmd);
	udelay(DELAY_TIME);
}

static void display(unsigned char dat)		   
{
	unsigned char i;

	write_cmd(0x08);		//显示模式：16位16段显示
	write_cmd(0x40);		//数据设置：地址增加
 	write_cmd(0xc0);		//地址设定：0xc0~0x0d对应地址为00H~0DH	
  
    for(i=0;i<16;i++)
    {
        write(dat);
        write(dat);
        write(0x00);
        write(0x00);
    }
    write_cmd(0x8F);		//显示控制：显示开
}

static void display_arr(unsigned char *dat)
{
    unsigned char i;

	write_cmd(0x08);		//显示模式：16位16段显示
	write_cmd(0x40);		//数据设置：地址增加
 	write_cmd(0xc0);		//地址设定：0xc0~0x0d对应地址为00H~0DH	
  
    for(i=0;i<32;i++)
    {
       write(*(dat++));                   //因为是16位16段，所以每写16个bit就要写两B的空数据去跳过没有用的段
       if(i%2==1){
         write(0x00);
         write(0x00);
       }
    }
    write_cmd(0x8F);		//显示控制：显示开
}

static void display_byBit(unsigned char addr, unsigned char dat)		   
{
	write_cmd(0x08);		//显示模式：16位16段显示
	write_cmd(0x44);		//数据设置：固定地址
        write_cmd(addr);		//地址设定：0xc0~0x0d对应地址为00H~0DH	
        write(dat);
	write_cmd(0x8F);		//显示控制：显示开
}

static int __init aip_init(void)
{
       int ret;
       int i;
       u32 GPIO119112_READ,GPIO119112_WRITE;
       GPIO119112_V = ioremap(GPIO119112,sizeof(u32));
       GPIO119112_READ = ioread32(GPIO119112_V);
       GPIO119112_WRITE = (GPIO119112_READ & 0x77770777);
       iowrite32(GPIO119112_WRITE,GPIO119112_V);

       u32 GPIO5548_READ,GPIO5548_WRITE;
       GPIO5548ADDR_V = ioremap(GPIO5548ADDR,sizeof(u32));
       GPIO5548_READ = ioread32(GPIO5548ADDR_V);
       GPIO5548_WRITE = (GPIO5548_READ & 0x77770077);
       iowrite32(GPIO5548_WRITE,GPIO5548ADDR_V);

       ret = misc_register(&aip_miscdev);
       if(ret < 0){
	     printk("misc device register failed!\r\n");
	     return -EFAULT;
       }

       misc_aip.clk_gpio = CLK_GPIO;
       misc_aip.dio_gpio = DIO_GPIO;
       misc_aip.stb_gpio = STB_GPIO;

       gpio_request(misc_aip.clk_gpio, "clk");
       gpio_request(misc_aip.dio_gpio, "dio");
       gpio_request(misc_aip.stb_gpio, "stb");

       ret = gpio_direction_output(misc_aip.clk_gpio, 0);
       if(ret < 0) {
		printk("can't set clk gpio!\r\n");
       }    
 
       ret = gpio_direction_output(misc_aip.dio_gpio, 0);
       if(ret < 0) {
	     printk("can't set dio gpio!\r\n");
       }

       ret = gpio_direction_output(misc_aip.stb_gpio, 0);
       if(ret < 0) {
		printk("can't set stb gpio!\r\n");
       }

       display(0xFF);
       mdelay(10);
       display(0x00);

       return 0;
}

static void __exit aip_exit(void)
{
        write_cmd(0x89);		//显示控制：显示关
        iounmap(GPIO119112_V);
        iounmap(GPIO5548ADDR_V);

       misc_deregister(&aip_miscdev);

       gpio_free(misc_aip.clk_gpio);
       gpio_free(misc_aip.dio_gpio);
       gpio_free(misc_aip.stb_gpio);
}

module_init(aip_init);
module_exit(aip_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");

