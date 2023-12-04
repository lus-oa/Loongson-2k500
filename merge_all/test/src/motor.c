// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include "motor.h"

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <sys/ioctl.h>

static char filename[] = "/dev/motor";
static int fd = -1;

int open_motor(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_motor(void)
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

int motor_control(unsigned int cmd)
{
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return -1;
	}
	if (ioctl(fd, cmd) < 0)
	{
		printf("电机模式切换失败\n");
		return -1;
	}
	return 0;
}

void motor_standby(void)
{
	motor_control(CMD_standby);
}

void motor_forward(void)
{
	motor_control(CMD_forward);
}

void motor_backward(void)
{
	motor_control(CMD_backward);
}

void motor_brake(void)
{
	motor_control(CMD_brake);
}

