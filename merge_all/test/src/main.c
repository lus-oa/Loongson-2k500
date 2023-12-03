#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <math.h> 
#include <errno.h> 
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h> 

#include "aht20.h"
#include "stk8ba.h"
#include "motor.h"
#include "beep.h"
#include "ch422g.h"
#include "rc522.h"
#include "aip1944.h"
#include "key16.h"
#include "led.h"

#define msleep(ms)	usleep(1000 * (ms))

pthread_t logo_pth;
int xyz[3], tem, hum;

typedef enum key_op {
	OP_LED1,
	OP_LED2,
	OP_LED3,
	OP_SHOW_TEM,
	OP_SHOW_HUM,
	OP_ERROR
} System_OP_Type;

enum {
	CH422G_NOTHTING,
	CH422G_TEM,
	CH422G_HUM
} ch422g_mode;

void system_init(void);
void system_exit(void);
System_OP_Type key_decode(uint16_t key);
void key_event(uint16_t key);
void ch422g_showdata(void);

void *logo_display(void *arg)
{
	//收到cancel信号，取消此线程
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //立即取消此线程
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	while (1)
	{
		aip1944_display(aip1944_demo,sizeof(aip1944_demo),AIP1944_SLIDE_MODE);
		aip1944_display_clear();
		aip1944_display(aip1944_demo,sizeof(aip1944_demo),AIP1944_ROLL_MODE);
		aip1944_display_clear();
	}
}

int main(int argc, char *argv[])
{
	int i;
	char id[RC522_ID_SIZE], rcbuf[RC522_BLOCK_SIZE];
	unsigned char aipbuf[32] = { 0 };
	uint16_t key;

	system_init();

	while (1)
	{
		key = key_scan();
		if (key)
		{
			printf("%x\n", key);
			sleep(1);
		}
		if (key & KEY16(15))
			break;
		key_event(key);
		read_aht20(&tem, &hum);
		//printf("温度：%.1f, 湿度：%.1f\n", tem/10.0, hum/10.0);
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
	
	ret = pthread_create(&logo_pth, NULL, logo_display, NULL);
	if (ret)
	{
		printf("logo初始化失败！\n");
		return;
	}

	printf("系统初始化成功\n");
}
void system_exit(void)
{
	pthread_cancel(logo_pth);
	pthread_join(logo_pth, NULL);

	close_stk8ba();
	close_aht20();
	close_beep();
	close_motor();
	close_aip1944();
	close_rc522();
	close_ch422g();
	close_key16();
	close_led();
	
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
	return OP_ERROR;
}

void key_event(uint16_t key)
{
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
	}
}

