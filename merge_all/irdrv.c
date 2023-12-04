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
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/input.h>
#include "key_code.h"

#define IR_GPIO 121

#define GPCFG 0x1fe104cc
#define KEYINPUT_NAME "keyinput" /* 名字 		*/

#define NEC_ENC 1
#ifdef NEC_ENC
#define START_TIME_MAX 11000
#define START_TIME_MIN 5000

#define L_TIME_MAX 660
#define L_TIME_MIN 360

#define L_TIME_MAX2 660
#define L_TIME_MIN2 360

#define H_TIME_MAX 1880
#define H_TIME_MIN 1480
#else
#define START_TIME_MAX 14500
#define START_TIME_MIN 11000

#define L_TIME_MAX 850
#define L_TIME_MIN 550

#define L_TIME_MAX2 750
#define L_TIME_MIN2 450

#define H_TIME_MAX 1900
#define H_TIME_MIN 1450
#endif

static void __iomem *GPCFG_V;

struct ir_dev {
	dev_t devid; /* 设备号 	 */
	struct cdev cdev; /* cdev 	*/
	struct class *class; /* 类 		*/
	struct device *device; /* 设备 	 */
	struct device_node *nd; /* 设备节点 */
	int major; /* 主设备号 */
	void *private_data; /* 私有数据 */
	int irq_num;
	struct input_dev *inputdev;
	int last_key_code;
};

static struct ir_dev my_dev;
static struct work_struct work;

static unsigned int get_key_table_id(unsigned int kv)
{
	int id;
	for (id = 0; id < KEY_NUM; id++) {
		if (kv == key_table[id].key_val) {
			if (KEY_NUM >= id)
				return id;
		}
	}
	return 0xFFFF;
}

struct infrared_pwm_t {
	long long previous; // 记录上一次的时间，64bit
	int flag; // 表示每个方波周期的开始
	long long start_time; // 周期的起始时间
	int low_time; // 低电平时间
	int high_time; // 高电平时间
};

struct infrared_pwm_t infrared_pwm = // 红外波形采集
	{
		.flag = 0,
		.previous = 0,
		.start_time = 0,
		.low_time = 0,
		.high_time = 0,
	};

/*------------------ 红外 NEC 数据解析结构体 ------------------*/
struct nec_decode_buf_t {
	int flag; // 表示 NEC 数据开始
	unsigned times[128]; // 记录每帧的时间
	int num; // 表示第几帧
};

struct nec_decode_buf_t nec_buf = {
	.flag = 0,
	.num = 0,
};

u8 InfraredRxBuff[5] = {
	0
}; //存放红外线接收的数据值，其中[4]表示标志位。=0失败，=1成功

static void infrared_nec_decode(int period, struct ir_dev *dev)
{
	int i, j;
	unsigned char temp;
	int repeat = 0;
	int id = 0xFFFF;

	if ((period > 13000) && (period < 14000)) {
		nec_buf.flag = 1;
		nec_buf.num = 0;
		repeat = 0;
		dev->last_key_code = 0;
		return;
	}

	if (nec_buf.num < 32) {
		nec_buf.times[nec_buf.num++] = period;
	}

	if (nec_buf.num == 32 && nec_buf.flag) {
		for (i = 0; i < 4; i++) // 一共4个字节
		{
			temp = 0;
			for (j = 0; j < 8; j++) {
				if ((nec_buf.times[i * 8 + j] > 2100) &&
				    (nec_buf.times[i * 8 + j] < 2500)) {
					temp |= 1 << j;
				}
			}
			InfraredRxBuff[i] = temp;
		}

		id = get_key_table_id(InfraredRxBuff[2]);
		dev->last_key_code = id;
		printk("Rx: %02x  %02x  %02x  %02x", InfraredRxBuff[0],
		       InfraredRxBuff[1], InfraredRxBuff[2], InfraredRxBuff[3]);
		printk("id: %d\r\n", id);
		nec_buf.flag = 0;
	}

	if ((period > 10500) && (period < 13500)) {
		if (!nec_buf.flag) {
			// printk(PRINTK_GRADE "Repetitive signal\n");
			memset(InfraredRxBuff, 0xFF, sizeof(InfraredRxBuff));
			repeat = 1;
		}
	}
	if (repeat) {
		id = dev->last_key_code;
	}
	if (id != 0xFFFF) {
		input_report_key(my_dev.inputdev, key_table[id].key_code,
				 1); //report press
		udelay(20);
		input_report_key(my_dev.inputdev, key_table[id].key_code,
				 0); //report release
		input_sync(my_dev.inputdev);
	}
	
}

/*
工作函数
*/
static irqreturn_t infrared_interrupt(int irq, void *dev)
{
	unsigned previous_offset; // 上一次的时间
	unsigned start_offset; // 波型的起始时间差
	long long now = ktime_to_us(ktime_get());

	/* 由于红外接收传感器在接收到红外信号时产生一个38KHz的信号，所以接收时需要过滤，使信号变为一个低电平信号 */
	/*-------------------------------- 滤波 --------------------------------*/

	/* 从当前时刻开始接收一个下降沿开始的方波周期 */
	if (0 == infrared_pwm.flag) {
		infrared_pwm.start_time = now;
		infrared_pwm.flag = 1;
	}

	/* 计算两次下降沿的时差 */
	previous_offset = now - infrared_pwm.previous;
	infrared_pwm.previous = now;

	/* 过滤红外接收器自生产生38KHz的信号，周期大约是 26us */
	if (previous_offset < 60) {
		return IRQ_HANDLED;
	}

	/* 下降沿开始的时差，也就是一个周期的时间 */
	start_offset = now - infrared_pwm.start_time;
	/* 消除上次持续的信号 */
	if (start_offset == 0) {
		return IRQ_HANDLED;
	}

	/* 完成一个周期的数据采集 */
	infrared_pwm.flag = 0;

	// infrared_pwm.low_time = start_offset - previous_offset + 52;		// 低电平时间
	// infrared_pwm.high_time = previous_offset - 52;					// 高电平时间

	/* NEC 解码 */
	infrared_nec_decode(start_offset, dev);

	return IRQ_HANDLED;
}

//DECLARE_WORK(infrared_work,infrared_work_func);

static int ir_init(void)
{
	unsigned char i = 0;
	char name[10];
	int ret = 0;
	u32 GPCFG_read, GPCFG_write;

	GPCFG_V = ioremap(GPCFG, sizeof(u32));
	GPCFG_read = ioread32(GPCFG_V);
	GPCFG_write = (GPCFG_read & 0x77777707);
	iowrite32(GPCFG_write, GPCFG_V);

	//INIT_WORK(&work,infrared_work_func);

	ret = gpio_request(IR_GPIO, "ir_gpio");
	pr_info("ir_init======ret:%d\r\n", ret);

	gpio_direction_input(IR_GPIO);
	my_dev.irq_num = gpio_to_irq(IR_GPIO);

	//|IRQF_TRIGGER_RISING
	ret = request_irq(my_dev.irq_num, infrared_interrupt,
			  IRQF_TRIGGER_RISING, "ir-irq",
			  &my_dev); //IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING
	if (ret < 0) {
		printk("irq %d request failed!\r\n", my_dev.irq_num);
		return -EFAULT;
	}

	my_dev.inputdev = input_allocate_device();
	my_dev.inputdev->name = KEYINPUT_NAME;
	my_dev.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);

	for (i = 0; i < KEY_NUM; i++) {
		input_set_capability(my_dev.inputdev, EV_KEY,
				     key_table[i].key_code);
	}

	/* 注册输入设备 */
	ret = input_register_device(my_dev.inputdev);
	if (ret) {
		printk("register input device failed!\r\n");
		return ret;
	}

	return 0;
}

static void __exit ir_exit(void)
{
	free_irq(my_dev.irq_num, &my_dev);
	gpio_free(IR_GPIO);
	iounmap(GPCFG_V);

	/* 释放input_dev */
	input_unregister_device(my_dev.inputdev);
	input_free_device(my_dev.inputdev);
}

module_init(ir_init);
module_exit(ir_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");
