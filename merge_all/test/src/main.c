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
#include "aip1944.h"

#define msleep(ms)	usleep(1000 * (ms))

int main(int argc, char *argv[])
{
	int xyz[3];
	int i, tem, hum;
	open_stk8ba();
	open_aht20();
	open_beep();
	open_motor();
	// open_aip1944();
	// display_sdu();
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
	for (i = 0; i < 10; i++)
	{
		read_stk8ba_xyz(xyz);
		printf("x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
		read_aht20(&tem, &hum);
		printf("温度：%.1f, 湿度：%.1f\n", tem/10.0, hum/10.0);
		msleep(500);
	}
	close_aip1944();
	close_stk8ba();
	close_aht20();
	close_beep();
	close_motor();
	return 0;
}








