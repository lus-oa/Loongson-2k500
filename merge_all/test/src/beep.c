#include "beep.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

static char filename[] = "/dev/beep";
static int fd = -1;

int open_beep(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_beep(void)
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


void beep_on(void)
{
	int ret;

	if (fd < 0)
	{
		printf("没有打开文件：%s\n", filename);
		return;
	}

	ret = ioctl(fd, BEEP_ON);
	if (ret < 0)
	{
		printf("蜂鸣器开启失败\n");
	}
}

void beep_off(void)
{
	int ret;

	if (fd < 0)
	{
		printf("没有打开文件：%s\n", filename);
		return;
	}

	ret = ioctl(fd, BEEP_OFF);
	if (ret < 0)
	{
		printf("蜂鸣器关闭失败\n");
	}
}

