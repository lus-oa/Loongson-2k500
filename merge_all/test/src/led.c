#include "led.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static char filename[] = "/dev/led";
static int fd = -1;
static unsigned char databuf[3] = { 0, 0, 0 };

int open_led(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_led(void)
{
	int ret;	
	ret = close(fd);
	if (ret < 0)
	{
		printf("can't close file %s\n", filename);
		return -1;
	}
	fd = -1;
	return ret;
}

int led_flush(void)
{
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return -1;
	}
	if (write(fd, databuf, sizeof(databuf)))
	{
		printf("LED写入失败\n");
		return -1;
	}
	return 0;
}

void led_control(unsigned char *buf)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		if (buf[i] <= 1)
			databuf[i] = buf[i];
		else if (buf[i] <= '1' && buf[i] >= '0')
			databuf[i] = buf[i] - '0';
		else
			databuf[i] = 0;
	}
	
	led_flush();
}

void led1_on(void)
{
	databuf[0] = 1;
	led_flush();
}

void led1_off(void)
{
	databuf[0] = 0;
	led_flush();
}

void led2_on(void)
{
	databuf[1] = 1;
	led_flush();
}

void led2_off(void)
{
	databuf[1] = 0;
	led_flush();
}

void led3_on(void)
{
	databuf[2] = 1;
	led_flush();
}

void led3_off(void)
{
	databuf[2] = 0;
	led_flush();
}

