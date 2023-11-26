#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include "rc522_api.h"

#define CHANGE_PASSWD 1
#define CHANGE_BLOCK  2
#define READ_CARD     3  
#define WRITE_CARD    4
#define CHANGE_KEY    5
#define GET_ID 	      6

#define GPCFG1	0x1fe104cc	//120-127
#define GPCFG2	0x1fe104d0	//128-135

static void __iomem *GPCFG1_V;
static void __iomem *GPCFG2_V;

typedef unsigned char uchar;
uchar NewKey[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x80,0x69,0x00,0x00,0x00,0x00,0x00,0x00};

/*static DECLARE_WAIT_QUEUE_HEAD(rc522_wait);*/
static unsigned char Read_Data[16]={0x00};
static unsigned char read_data_buff[16];

static uchar PassWd[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static uchar WriteData[16];
static unsigned char RevBuffer[30];
static unsigned char MLastSelectedSnr[4];

uint KuaiN;
uchar operationcard;

void InitRc522(void)
{
	unsigned char a;
	PcdReset();
	a = ReadRawRC(TReloadRegL);
	
	if(a != 30)
		pr_info("What?, NO RC522:%d\n",a);
	else
		pr_info("RC522 exist\n");
	PcdAntennaOff();  
	PcdAntennaOn();
	M500PcdConfigISOType( 'A' );
}

static char rc522_loop_work(uchar opnd)
{
	char *pdata = read_data_buff;
	char status;
	int t = 0;

	while ((status = PcdRequest(PICC_REQIDL,&RevBuffer[0])) != MI_OK)
	{
		PcdReset();
        	PcdAntennaOff(); 
        	mdelay(1);
        	PcdAntennaOn();
		t++;
		if (t > 3)
		{
			pr_info("search card: no card\n");
			return -EFAULT;
		}
	}

	status=PcdAnticoll(&RevBuffer[2]);
	if(status!=MI_OK)
	{
		pr_info("get card nu: no number\n");
		return -EFAULT;
	} 
	memcpy(MLastSelectedSnr,&RevBuffer[2],4);

	status=PcdSelect(MLastSelectedSnr);
	if(status!=MI_OK)
	{
		pr_info("select card: no card\n");
		return -EFAULT;            
	}
	if (opnd == GET_ID) {
		PcdHalt();
		return 0;
	}
	else if (opnd == READ_CARD) {
		status=PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			pr_info("read authorize card err\n");
			return -EFAULT;
		}
		status=PcdRead(KuaiN,Read_Data);
		if(status!=MI_OK)
		{
			pr_info("read card err\n");
			return -EFAULT;
		} else {
			int i;
			memcpy(pdata, Read_Data, sizeof(Read_Data));
			/*wake_up_interruptible(&rc522_wait);*/
			pr_info("read block %d info:", KuaiN);
			for(i = 0; i < 16; i++) {
				pr_info("%2.2X",pdata[i]);
			}
			pr_info("\n");
		}
	} else if (opnd == CHANGE_KEY) {//ÐÞ¸ÄÃÜÂë
		status=PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			pr_info("card authorize err\n");
			return -EFAULT;
		}
		status=PcdWrite(KuaiN,&NewKey[0]);
		if(status!=MI_OK)
		{
			pr_info("change password err\n");
			return -EFAULT;
		} else
			pr_info("set password success\n");
	} else if (opnd == WRITE_CARD) {//Ð´¿¨
		pr_info("write info:%s\n", WriteData);
		status=PcdAuthState(PICC_AUTHENT1A,KuaiN,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			pr_info("write authrioze err\n");
			return -EFAULT;
		}
		status=PcdWrite(KuaiN,&WriteData[0]);
		if(status!=MI_OK)
		{
			pr_info("write data err\n");
			return -EFAULT;
		} else {
			pr_info("write data to block %d sucess\n", KuaiN);
		}
	}
	PcdHalt();	
	return 0;
}



static int rc522_open(struct inode *inode,struct file *filp)
{

	InitRc522();
	pr_info("rc522 start work!\n");
	return 0;
}

static ssize_t rc522_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	/*PcdReset();*/
	operationcard = READ_CARD;
	if(rc522_loop_work(operationcard))
		return 0;
	pr_info("card info:%2.2X\n",Read_Data[0]);
	if (copy_to_user(buf, read_data_buff, sizeof(read_data_buff))) {
		pr_info("copy card number to userspace err\n");
		return 0;
	}
	return sizeof(read_data_buff);
}

static ssize_t rc522_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	if (KuaiN == 0) {
		pr_info("block[0] is reserveed, can't write\n");
		return 0;
	}
	if (KuaiN < 0 || KuaiN > 63) {
		pr_info("block[%d] unreachable, please set the write block first", KuaiN);
		return -0;
	} 
	if ((KuaiN % 4) == 3) {
		pr_info("block[%d] is key block, not data block\n", KuaiN);
		return -0;
	}
	memset(WriteData, 0, sizeof(WriteData));
	if (copy_from_user(WriteData, (char *)buf, count)) {
		pr_info("%s, [line %d] copy from user err.\n", __FILE__, __LINE__);
		return 0;
	}
	/*PcdReset();*/
	operationcard =  WRITE_CARD;
	pr_info("writing...\n");
	if(rc522_loop_work(operationcard))
		return -EFAULT;
	return 0;
}

static int rc522_release(struct inode *inode,struct file *filp)
{
	pr_info("%s\n", __func__);
	return 0;
}


static long rc522_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	switch(cmd) {
		case CHANGE_PASSWD:
			operationcard = CHANGE_PASSWD;
			if (copy_from_user(PassWd, (char *)arg, sizeof(PassWd))) {
				pr_info("%s:change pass word err", __func__);
				return -EFAULT;
			}
			break;
		case CHANGE_BLOCK:
			if (arg < 0 || arg > 63) {
				pr_info("block number err %lu", arg);
				return -EFAULT;
			}
			KuaiN = (int)arg;
			break;
		case READ_CARD:
			break;
		case WRITE_CARD:
			break;
		case CHANGE_KEY:
			operationcard = CHANGE_KEY;
			break;
		case GET_ID:
			operationcard =  GET_ID;
			if(!rc522_loop_work(operationcard)){
				if (copy_to_user((char *)arg, MLastSelectedSnr,4)) {
					pr_info("%s, [line %d] copy to user err.\n", __FILE__, __LINE__);
					return -EFAULT;
				}
			}
			else
				return -EFAULT;
			break;
		default:
			break;
	}
	return 0;
}

static struct file_operations rc522_fops = {
	.owner = THIS_MODULE,
	.open = rc522_open,
	.release = rc522_release, 
	.read = rc522_read,
	.write = rc522_write,
	.unlocked_ioctl = rc522_ioctl,
};

static struct miscdevice rc522_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "rfid_dev",
	.fops = &rc522_fops,
};

static int __init RC522_init(void)
{
	int ret;
	u32 GPCFG1_read, GPCFG1_write;
	u32 GPCFG2_read, GPCFG2_write;

	/* Register the character device (atleast try) */
	pr_info("rfid_rc522 module init.\n");

	GPCFG1_V = ioremap(GPCFG1, sizeof(u32));
	GPCFG1_read = ioread32(GPCFG1_V);
	GPCFG1_write = (GPCFG1_read & 0x00077777);
	iowrite32(GPCFG1_write, GPCFG1_V);

	GPCFG2_V = ioremap(GPCFG2, sizeof(u32));
	GPCFG2_read = ioread32(GPCFG2_V);
	GPCFG2_write = (GPCFG2_read & 0x77777700);
	iowrite32(GPCFG2_write, GPCFG2_V);
	
        gpio_request(GPIO_RST, "rst");
        ret = gpio_direction_output(GPIO_RST, 0);
        if(ret < 0) {
		printk("can't set rst gpio!\r\n");
                return ret;
        }
	
	gpio_request(GPIO_CS, "cs");
	ret = gpio_direction_output(GPIO_CS, 1);
        if(ret < 0) {
		printk("can't set rst gpio!\r\n");
                return ret;
        }

	gpio_request(GPIO_CLK, "clk");
	ret = gpio_direction_output(GPIO_CLK, 1);
        if(ret < 0) {
		printk("can't set rst gpio!\r\n");
                return ret;
        }

	gpio_request(GPIO_MISO, "miso");
	ret = gpio_direction_input(GPIO_MISO);
	if(ret < 0) {
		printk("can't set miso gpio!\r\n");
                return ret;
        }

	gpio_request(GPIO_MOSI, "mosi");
	ret = gpio_direction_output(GPIO_MOSI, 1);
	if(ret < 0) {
		printk("can't set mosi gpio!\r\n");
                return ret;
        }

	ret =  misc_register(&rc522_misc_device);
	if(ret < 0) {
		pr_info("device register failed with %d.\n",ret);
		return ret;
	}

	KuaiN = 1;
	PcdReset();
	PcdAntennaOff(); 	// 关闭天线
	mdelay(10);	
	PcdAntennaOn();		// 开启天线
	mdelay(10);

	return 0;
}

static void __exit RC522_exit(void)
{
	pr_info("rc522 driver exit\n");

	iounmap(GPCFG1_V);
	iounmap(GPCFG2_V);	

        gpio_free(GPIO_RST);
	gpio_free(GPIO_CS);
	gpio_free(GPIO_CLK);
	gpio_free(GPIO_MISO);
	gpio_free(GPIO_MOSI);
	
	misc_deregister(&rc522_misc_device);
}

module_init(RC522_init);
module_exit(RC522_exit);

MODULE_AUTHOR("wugang");
MODULE_LICENSE("Dual BSD/GPL");	
