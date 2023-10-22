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

#define CH422G_NAME		"ch422g"	/* 名字 	*/
#define CH422G_MINOR		146			/* 子设备号 */

#define SDA3_GPIO                142
#define SCL3_GPIO                141
#define SPACE2                    0x10       // 空格
#define CH422_SCL_D_OUT     { } 
#define CH422_SDA_D_OUT     { }  
#define DELAY_TIME          10

#define GPIO_142_INDEX 	1 << ((SDA3_GPIO % 32) + 0)
#define GPIO_141_INDEX 	1 << ((SCL3_GPIO % 32) + 0)

#define GPIO_SET_ALL_LEDS (GPIO_141_INDEX  | GPIO_142_INDEX)

#define GPFSET 	 0x1fe10480  //0x1fe10460
#define GPIN   	 0x1fe10478  //0x1fe10458
#define GPCTR0 	 0x1fe10470  //0x1fe10450
#define GPMUX    0x1fe104d4  //0x1fe104b0

/* __iomem  虚拟地址指针，保存虚拟地址*/
static void __iomem *GPSET0_V;
static void __iomem *GPIN0_V;
static void __iomem *GPCTR0_V;
static void __iomem *GPMUX0_V;

#define CLEAR_CMD     0x01

struct ch422g_dev{
	dev_t devid;			/* 设备号 	 */
        void *private_data;	/* 私有数据 */
        int sda_gpio;
        int scl_gpio;
};

struct ch422g_dev misc_ch422g;

const unsigned char BCD_decode_tab[ 29 ] = { 0X3F, 0X06, 0X5B, 0X4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F, 0X77, 0X7C, 0X58, 0X5E, 0X79, 0X71, 0x00, 0x46, 0x40, 0x41, 0x39, 0x0F, 0x08, 0x76, 0x38, 0x73, 0x80, 0xFF, 0x00 };

unsigned char CH422_buf[ 16 ];    // 定义16个数码管的数据映象缓存区

static void CH422_buf_index( unsigned char index, unsigned char dat )
{
    if( index >= sizeof( CH422_buf ) ) index = 0;
    if( dat >= sizeof( BCD_decode_tab ) ) dat = SPACE2;
    CH422_buf[index ] = BCD_decode_tab[ dat ];
}

static void CH422_set_bit( unsigned char bit_addr )    // 段位点亮
{
    unsigned char byte_addr;
    byte_addr = ( bit_addr>>3 ) & 0x0F;
    CH422_buf[ byte_addr ] |= 1<<( bit_addr&0x07 );
}

static void CH422_SCL_SET(void)
{
    gpio_set_value(misc_ch422g.scl_gpio, 1);
}

static void CH422_SCL_CLR(void)
{
     u32 GPFSEL_read, GPFSEL_write;

     gpio_set_value(misc_ch422g.scl_gpio, 0);
}

static void CH422_SDA_SET(void)
{
    gpio_set_value(misc_ch422g.sda_gpio, 1);
}

static void CH422_SDA_CLR(void)
{
    gpio_set_value(misc_ch422g.sda_gpio, 0);
}

void CH422_I2c_Start( void )    // 操作起始
{
    CH422_SDA_SET();    // 发送起始条件的数据信号
    CH422_SDA_D_OUT;    // 设置SDA为输出方向 
    CH422_SCL_SET();
    CH422_SCL_D_OUT;    // 设置SCL为输出方向 
    udelay(DELAY_TIME);
    CH422_SDA_CLR();    //发送起始信号
    udelay(DELAY_TIME);      
    CH422_SCL_CLR();    //钳住I2C总线，准备发送或接收数据 
}

void CH422_I2c_Stop( void )    // 操作结束
{
    CH422_SDA_CLR();
    udelay(DELAY_TIME);
    CH422_SCL_SET();
    udelay(DELAY_TIME);
    CH422_SDA_SET();    // 发送I2C总线结束信号
    udelay(DELAY_TIME);
}

void CH422_I2c_WrByte( unsigned char dat )    // 写一个字节数据
{
    unsigned char i;
    for( i = 0; i != 8; i++ )    // 输出8位数据
    {
        if( dat&0x80 ) { CH422_SDA_SET(); }
        else { CH422_SDA_CLR(); }
        udelay(DELAY_TIME);
        CH422_SCL_SET();
        dat<<=1;
        udelay(DELAY_TIME);    // 可选延时
        CH422_SCL_CLR();
    }
    CH422_SDA_SET();
    udelay(DELAY_TIME);
    CH422_SCL_SET();    // 接收应答
    udelay(DELAY_TIME);
    CH422_SCL_CLR();
}

void CH422_WriteByte( unsigned short cmd )    // 写出数据
{
    CH422_I2c_Start();    // 启动总线
    CH422_I2c_WrByte( ( unsigned char )( cmd>>8 ) );
    CH422_I2c_WrByte( ( unsigned char ) cmd );    // 发送数据
    CH422_I2c_Stop();    // 结束总线  
}

static int ch422g_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &misc_ch422g; /* 设置私有数据 */
	return 0;
}

static ssize_t ch422g_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[5];
	struct dac_dev *dev = filp->private_data;
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

        CH422_WriteByte( CH422_SET_IO_CMD + (0x07)); //0111
        CH422_WriteByte( CH422_DIG0 | ch1 );

        CH422_WriteByte( CH422_SET_IO_CMD + (0x0b)); //1011
        CH422_WriteByte( CH422_DIG1 | ch2 );

        CH422_WriteByte( CH422_SET_IO_CMD + (0x0d)); //1101
        CH422_WriteByte( CH422_DIG2 | ch3 );

        CH422_WriteByte( CH422_SET_IO_CMD + (0x0e)); //1110
        CH422_WriteByte( CH422_DIG3 | ch4 );

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
        int ret,j;
        unsigned char i,ch1,ch2,ch3,ch4;
        u32 GPFSEL_read, GPFSEL_write;
	u32 GPFCTR_read, GPFCTR_write;
        u32 GPFMUX_read, GPFMUX_write;

        ret = misc_register(&ch422g_miscdev);
	if(ret < 0){
		printk("misc device register failed!\r\n");
		return -EFAULT;
	}

        GPSET0_V = ioremap(GPFSET, sizeof(u32));
	GPIN0_V = ioremap(GPIN, sizeof(u32));
	GPCTR0_V = ioremap(GPCTR0, sizeof(u32));
        GPMUX0_V = ioremap(GPMUX, sizeof(u32));
	
	pr_info("ch422g_init===>GPSET0_V:%x,---GPIN0_V:%x,---GPCTR0_V:%x,---GPMUX0_V%x\n",GPSET0_V,GPIN0_V,GPCTR0_V,GPMUX0_V);

        GPFSEL_read = ioread32(GPSET0_V);
	GPFCTR_read = ioread32(GPCTR0_V);
        GPFMUX_read = ioread32(GPMUX0_V);
        pr_info("ch422g_init1===>GPFSEL_read:%x,---GPFCTR_read:%x,---GPFMUX_read:%x\n",GPFSEL_read,GPFCTR_read,GPFMUX_read);

        GPFMUX_write = (GPFMUX_read & 0x70077777);
	GPFSEL_write = (GPFSEL_read & (~GPIO_SET_ALL_LEDS));
	GPFCTR_write = (GPFCTR_read & (~GPIO_SET_ALL_LEDS));

        iowrite32(GPFMUX_write, GPMUX0_V);

        misc_ch422g.sda_gpio = SDA3_GPIO;
        misc_ch422g.scl_gpio = SCL3_GPIO;

        gpio_request(misc_ch422g.sda_gpio, "sda3");
        gpio_request(misc_ch422g.scl_gpio, "scl3");
        ret = gpio_direction_output(misc_ch422g.sda_gpio, 0);
	if(ret < 0) {
		printk("can't set sda gpio!\r\n");
	}    
 
        ret = gpio_direction_output(misc_ch422g.scl_gpio, 0);
	if(ret < 0) {
		printk("can't set scl gpio!\r\n");
	}

        //CH422_WriteByte( CH422_SYS_CMD | BIT_DEC_L | BIT_IO_OE );    // 设置系统参数命令
        CH422_WriteByte( CH422_SYS_CMD | 0x5 );
  
        //CH422_WriteByte( 0x4600 | 0x0f);
        //msleep( 1 );
        //CH422_WriteByte( 0x4600 | 0x00);

        
        return ret;
}

static void __exit ch422g_exit(void)
{

        iounmap(GPMUX0_V);
        iounmap(GPSET0_V);
	iounmap(GPIN0_V);
	iounmap(GPCTR0_V);

       misc_deregister(&ch422g_miscdev);
       gpio_free(misc_ch422g.sda_gpio);
       gpio_free(misc_ch422g.scl_gpio);
}

module_init(ch422g_init);
module_exit(ch422g_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");

