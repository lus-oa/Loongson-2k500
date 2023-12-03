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

void system_init(void);
void system_exit(void);

int main(int argc, char *argv[])
{
	int xyz[3];
	int i, tem, hum;
	char id[RC522_ID_SIZE], rcbuf[RC522_BLOCK_SIZE];
	unsigned char aipbuf[32] = { 0 };

	system_init();

#if 1
	aip1944_display_clear();
	aipbuf[0] = 0xF0;
	aipbuf[1] = 0x55;
	aip1944_set_data(aipbuf);
	sleep(2);
#endif

#if 1
	aip1944_display(aip1944_demo,sizeof(aip1944_demo),AIP1944_SLIDE_MODE);
	aip1944_display_clear();
	aip1944_display(aip1944_demo,sizeof(aip1944_demo),AIP1944_ROLL_MODE);
	aip1944_display_clear();
#endif
#if 0
	// 从左到右递增1，从上到下递增4，bit代表当前按的位，暂时不支持同行一起按
	uint16_t key = key_scan();
	for (int i = 0; i < 16; i++) {
		printf("%d", (key >> i) & 1);
	}
	printf("\n");
#endif
#if 0	//太吵了
	beep_on();
	msleep(200);
	beep_off();
	motor_standby();
	msleep(500);
	motor_forward();
	msleep(500);
	motor_backward();
	msleep(500);
	motor_brake();
#endif
#if 0
	rc522_getid(id);
	printf("%d%d%d%d\n", id[0], id[1], id[2], id[3]);
	while (1)
	{
		if (rc522_read(rcbuf) == sizeof(rcbuf))
		{
			for (i = 0; i < sizeof(rcbuf); i++)
				printf("%x ", rcbuf[i]);
			printf("\n");
			rc522_write("dfs134", 6);
			rc522_read(rcbuf);
			for (i = 0; i < sizeof(rcbuf); i++)
				printf("%x ", rcbuf[i]);
			printf("\n");
			break;
		}
		sleep(1);
	}
#endif
#if 0
	while (1)
	{
		uint16_t key = key_scan();
		if (key & KEY16(15))
			break;
		if (key & KEY16(0))
			led1_on();
		if (key & KEY16(1))
			led2_on();
		if (key & KEY16(2))
			led3_on();
		if (key & KEY16(3))
			led_control("111");
		if (key & KEY16(4))
			led_control("000");
	}
#endif

#if 1
	for (i = 0; i < 10; i++)
	{
		read_stk8ba_xyz(xyz);
		printf("x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
		read_aht20(&tem, &hum);
		printf("温度：%.1f, 湿度：%.1f\n", tem/10.0, hum/10.0);
		ch422g_set_num(2, tem / 100);
		ch422g_set_num(3, tem / 10);
		ch422g_set_mask(3, CH422G_PT);
		ch422g_set_num(4, tem);
		ch422g_flush();
		msleep(500);
	}
#endif
	
	system_exit();
	return 0;
}

void system_init(void)
{
	open_stk8ba();
	open_aht20();
	open_beep();
	open_motor();
	open_aip1944();
	open_rc522();
	open_ch422g();
	//open_key16();
}
void system_exit(void)
{
	
	close_stk8ba();
	close_aht20();
	close_beep();
	close_motor();
	close_aip1944();
	close_rc522();
	close_ch422g();
	//close_key16();
}

