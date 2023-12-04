#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "aht20.h"
#include "stk8ba.h"
#include "motor.h"
#include "beep.h"
#include "ch422g.h"
#include "rc522.h"
#include "aip1944.h"
#include "key16.h"
#include "led.h"
#include "ir.h"

#define msleep(ms)	usleep(1000 * (ms))

struct ir_info_struct {
	struct input_event ir_event;
	int ir_flag;
	pthread_mutex_t ir_lock;
};

typedef enum key_op {
	OP_LED1,
	OP_LED2,
	OP_LED3,
	OP_SHOW_TEM,
	OP_SHOW_HUM,
	OP_BEEP,
	OP_MOTOR_STANDBY,
	OP_MOTOR_FORWARD,
	OP_MOTOR_BACKWARD,
	OP_MOTOR_BRAKE,
	OP_ERROR
} System_OP_Type;

pthread_t logo_pth, ir_pth, rc_pth;
int logo_mode = AIP1944_SLIDE_MODE;
int xyz[3], tem, hum;
struct ir_info_struct ir_info;

enum {
	CH422G_NOTHTING,
	CH422G_TEM,
	CH422G_HUM
} ch422g_mode;

void system_init(void);
void system_exit(void);
System_OP_Type key_decode(uint16_t key);
void key_handler(uint16_t key);
void ir_handler(struct input_event *ir_event);
void ch422g_showdata(void);

void *logo_display(void *arg)
{
	//收到cancel信号，取消此线程
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //立即取消此线程
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	while (1)
	{
		aip1944_display(aip1944_demo,sizeof(aip1944_demo),logo_mode);
		aip1944_display_clear();
	}
}

void *ir_receive(void *arg)
{
	struct input_event eventbuf;

	//收到cancel信号，取消此线程
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //立即取消此线程
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while (1)
	{
		ir_read(&eventbuf);

		pthread_mutex_lock(&ir_info.ir_lock);
		if (!ir_info.ir_flag)
		{
			memcpy(&ir_info.ir_event, &eventbuf, sizeof(eventbuf));
			ir_info.ir_flag = 1;
		}
		pthread_mutex_unlock(&ir_info.ir_lock);
	}
}

void *rc522_handler(void *arg)
{
	char readbuf[RC522_BLOCK_SIZE];
	int ret;

	//收到cancel信号，取消此线程
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //立即取消此线程
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	

	while (1)
	{
		ret = rc522_read(readbuf);
		if (ret == sizeof(readbuf))
		{
			led_on(0);
		}
		else
		{
			led_off(0);
		}
	}
}

int main(int argc, char *argv[])
{
	uint16_t key;
	struct input_event ir_event;

	system_init();

	while (1)
	{
		key = key_scan();
		if (key & KEY16(15))
			break;

		key_handler(key);
		ir_handler(&ir_event);
		read_aht20(&tem, &hum);
		read_stk8ba_xyz(xyz);
		//printf("x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
		ch422g_showdata();
		
	}
	
	system_exit();
	return 0;
}

void system_init(void)
{
	int ret = 0;
	open_stk8ba();
	open_aht20();
	open_beep();
	open_motor();
	open_aip1944();
	open_rc522();
	open_ch422g();
	open_key16();
	open_led();
	open_ir();
	
	ret = pthread_create(&logo_pth, NULL, logo_display, NULL);
	if (ret)
	{
		printf("logo初始化失败！\n");
		return;
	}

	ret = pthread_create(&ir_pth, NULL, ir_receive, NULL);
	if (ret)
	{
		printf("红外处理初始化失败！\n");
		return;
	}

	ret = pthread_create(&rc_pth, NULL, rc522_handler, NULL);
	if (ret)
	{
		printf("智能卡初始化失败！\n");
		return;
	}

	printf("系统初始化成功\n");
}

void system_exit(void)
{
	pthread_cancel(logo_pth);
	pthread_cancel(ir_pth);
	pthread_cancel(rc_pth);
	pthread_join(logo_pth, NULL);
	pthread_join(ir_pth, NULL);
	pthread_join(rc_pth, NULL);

	close_stk8ba();
	close_aht20();
	close_beep();
	close_motor();
	close_aip1944();
	close_rc522();
	close_ch422g();
	close_key16();
	close_led();
	close_ir();
	
	printf("系统退出成功\n");
}

System_OP_Type key_decode(uint16_t key)
{
	if (key & KEY16(0))
		return OP_LED1;
	if (key & KEY16(1))
		return OP_LED2;
	if (key & KEY16(2))
		return OP_LED3;
	if (key & KEY16(3))
		return OP_SHOW_TEM;
	if (key & KEY16(4))
		return OP_SHOW_HUM;
	if (key & KEY16(5))
		return OP_BEEP;
	if (key & KEY16(6))
		return OP_MOTOR_STANDBY;
	if (key & KEY16(7))
		return OP_MOTOR_FORWARD;
	if (key & KEY16(8))
		return OP_MOTOR_BACKWARD;
	if (key & KEY16(9))
		return OP_MOTOR_BRAKE;
	
	return OP_ERROR;
}

void key_handler(uint16_t key)
{
	static int beep_is_on = 0;

	System_OP_Type op = key_decode(key);
	switch (op)
	{
	case OP_LED1:
		led_neg(0);
		break;
	case OP_LED2:
		led_neg(1);
		break;
	case OP_LED3:
		led_neg(2);
		break;
	case OP_SHOW_TEM:
		ch422g_mode = CH422G_TEM;
		break;
	case OP_SHOW_HUM:
		ch422g_mode = CH422G_HUM;
		break;
	case OP_BEEP:
		if (beep_is_on)
			beep_off();
		else
			beep_on();
		beep_is_on = beep_is_on ? 0 : 1;
		break;
	case OP_MOTOR_STANDBY:
		motor_standby();
		break;
	case OP_MOTOR_FORWARD:
		motor_forward();
		break;
	case OP_MOTOR_BACKWARD:
		motor_backward();
		break;
	case OP_MOTOR_BRAKE:
		motor_brake();
		break;
	case OP_ERROR:
		break;
	default:
		break;
	}
}

void ch422g_showdata(void)
{
	switch (ch422g_mode)
	{
	case CH422G_TEM:
		ch422g_set_num(2, tem / 100);
		ch422g_set_num(3, tem / 10);
		ch422g_set_mask(3, CH422G_PT);
		ch422g_set_num(4, tem);
		ch422g_flush();
		break;
	case CH422G_HUM:
		ch422g_set_num(2, hum / 100);
		ch422g_set_num(3, hum / 10);
		ch422g_set_mask(3, CH422G_PT);
		ch422g_set_num(4, hum);
		ch422g_flush();
		break;
	default:
		break;
	}
}

void ir_handler(struct input_event *ir_event)
{
	struct key_table_t ir_key;
	pthread_mutex_lock(&ir_info.ir_lock);
	if (ir_info.ir_flag)
	{
		ir_encode(&ir_info.ir_event, &ir_key);
		ir_info.ir_flag = 0;
	}
	else
	{
		return;
	}
	pthread_mutex_unlock(&ir_info.ir_lock);
	
	switch (ir_key.key_code)
	{
	case KEY_1:
		led_neg(0);
		led_neg(1);
		led_neg(2);
		break;
	case KEY_2:
		printf("key2\n");
		if (logo_mode == AIP1944_SLIDE_MODE)
			logo_mode = AIP1944_ROLL_MODE;
		else
			logo_mode = AIP1944_SLIDE_MODE;
		pthread_cancel(logo_pth);
		pthread_join(logo_pth, NULL);
		pthread_create(&logo_pth, NULL, logo_display, NULL);
		break;
	default:
		break;
	}
}

