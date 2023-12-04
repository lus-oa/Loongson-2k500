// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

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
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/input.h>
#include "key_code.h"

#define IRQ_CNT 1 /* 设备号个数 	*/
#define IRQ_NAME "ir_irq" /* 名字 		*/

#define IR_GPIO 121

#define EXTINT_IEN0 0x1fe11600
#define EXTINT_IEN1 0x1fe11608

#define EXTINT_ISR0 0x1fe11700
#define EXTINT_ISR1 0x1fe11708

#define GPMUX 0x1fe104cc //0x1fe104b0
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

static void __iomem *GPMUX0_V;

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

static u32 GetInfraredRxH(void)
{
	ktime_t time_val;
	unsigned int tim_cnt1 = 0, tim_cnt2 = 0;
	time_val = ktime_get(); //获取当前时间
	tim_cnt1 = ktime_to_us(time_val); //转us
	while (!gpio_get_value(IR_GPIO)) {
	}
	time_val = ktime_get(); //获取当前时间
	tim_cnt2 = ktime_to_us(time_val); //转us
	return tim_cnt2 - tim_cnt1;
}

static u32 GetInfraredRxL(void)
{
	ktime_t time_val;
	unsigned int tim_cnt1 = 0, tim_cnt2 = 0;
	time_val = ktime_get(); //获取当前时间
	tim_cnt1 = ktime_to_us(time_val); //转us
	while (gpio_get_value(IR_GPIO)) {
	}
	time_val = ktime_get(); //获取当前时间
	tim_cnt2 = ktime_to_us(time_val); //转us
	return tim_cnt2 - tim_cnt1;
}

/*
工作函数
*/
u8 InfraredRxBuff[5] = {
	0
}; //存放红外线接收的数据值，其中[4]表示标志位。=0失败，=1成功
void infrared_work_func(struct work_struct *dat)
{
	u32 time_l, time_h, j, i;
	u8 data = 0;
	int id = 0;
	printk(">c\r\n");

	/*1. 判断引导码*/
	time_l = GetInfraredRxL(); //获取低电平的时间 9000us
	printk(">L:%d\r\n",time_l);
	if ((time_l < START_TIME_MIN) || (time_l > START_TIME_MAX)){
		// goto out;
	}

	time_h = GetInfraredRxH(); //4500us
	printk(">H:%d\r\n",time_h);
	if ((time_h < 2500) || (time_h > 5500))
		goto out;

	/*2. 接收用户码和按键码*/
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			time_l = GetInfraredRxL(); //获取低电平的时间   560us
			// printk(">L:%d\r\n",time);
			if ((time_l < L_TIME_MIN) || (time_l > L_TIME_MAX))
				goto out;
			time_h = GetInfraredRxH(); //获取高电平的时间
			// printk(">H:%d\r\n",time);
			//560us高电平  0  、 1680us高电平 1
			if ((time_h > L_TIME_MIN2) &&
			    (time_h < L_TIME_MAX2)) //接到数据0
			{
				data >>= 1;
			} else if ((time_h > H_TIME_MIN) &&
				   (time_h < H_TIME_MAX)) //接收到数据1
			{
				data >>= 1;
				data |= 0x80; //1000 0000
			} else {
				goto out;
			}
		}
		InfraredRxBuff[i] = data;
	}
	InfraredRxBuff[4] = 1; //标志红外线解码成功

	//打印解码数据
	if (InfraredRxBuff[4]) {
		InfraredRxBuff[4] = 0; //清除接收成功标志
		id = get_key_table_id(InfraredRxBuff[2]);
		if (id != 0xFFFF) {
			input_report_key(my_dev.inputdev,
					 key_table[id].key_code,
					 1); //report press
			udelay(20);
			input_report_key(my_dev.inputdev,
					 key_table[id].key_code,
					 0); //report release
			input_sync(my_dev.inputdev);
		}

		printk("USER=0x%x,KEY=0x%x,id:%d\r\n", InfraredRxBuff[0],
		       InfraredRxBuff[2], id);

		//printk("KEY=0x%x\r\n",InfraredRxBuff[2]);
	}
out:
	udelay(80 * 1000);
	//if((i>0))
	//printk(">i:%d,j:%d,time_l:%d,time_h:%d\r\n",i,j,time_l,time_h);
	//使能中断
	enable_irq(my_dev.irq_num);
	//printk(">\r\n");
}

static irqreturn_t gpio_ir_recv_irq(int irq, void *dev_id)
{
	struct ir_dev *dev = (struct ir_dev *)dev_id;

	disable_irq_nosync(dev->irq_num);
	// schedule_work(&work);
	infrared_work_func(NULL);

	return IRQ_RETVAL(IRQ_HANDLED);
}

//DECLARE_WORK(infrared_work,infrared_work_func);

static int ir_init(void)
{
	unsigned char i = 0;
	char name[10];
	int ret = 0;
	u32 GPFMUX_read, GPFMUX_write;

	GPMUX0_V = ioremap(GPMUX, sizeof(u32));
	GPFMUX_read = ioread32(GPMUX0_V);
	GPFMUX_write = (GPFMUX_read & 0x77777707);
	iowrite32(GPFMUX_write, GPMUX0_V);

	INIT_WORK(&work, infrared_work_func);

	ret = gpio_request(IR_GPIO, "ir_gpio");
	pr_info("ir_init======ret:%d\r\n", ret);

	gpio_direction_input(IR_GPIO);
	my_dev.irq_num = gpio_to_irq(IR_GPIO);

	//|IRQF_TRIGGER_RISING
	ret = request_irq(my_dev.irq_num, gpio_ir_recv_irq, IRQF_TRIGGER_RISING,
			  "ir-irq",
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

	/* 释放input_dev */
	input_unregister_device(my_dev.inputdev);
	input_free_device(my_dev.inputdev);
}

module_init(ir_init);
module_exit(ir_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wugang");
