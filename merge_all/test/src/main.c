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

#define msleep(ms)	usleep(1000 * (ms))

void system_init(void);
void system_exit(void);

int main(int argc, char *argv[])
{
	int xyz[3];
	int i, tem, hum;
	char id[RC522_ID_SIZE], rcbuf[RC522_BLOCK_SIZE];

	system_init();

	display_sdu();
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
#if 1
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
#if 1
	ch422g_set_char(2, 'a');
	ch422g_set_tube(3, CH422G_M2 | CH422G_L1 | CH422G_R1);
	for (i = 0; i < 10; i++)
	{
		read_stk8ba_xyz(xyz);
		printf("x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
		read_aht20(&tem, &hum);
		printf("温度：%.1f, 湿度：%.1f\n", tem/10.0, hum/10.0);
		ch422g_set_char(1, i + '0');
		msleep(500);
	}
#endif
	display_clear();
	
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
}

