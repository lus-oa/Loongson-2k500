// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include "aht20.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static char filename[] = "/dev/aht20";
static int fd = -1;

int open_aht20(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_aht20(void)
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


int read_aht20_origin(unsigned char *databuf)
{
	int ret;
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return -1;
	}

	ret = read(fd, databuf, AHT20_ORI_SIZE);
	if (ret != AHT20_ORI_SIZE || (databuf[0] & 0x68) != 0x08)
	{
		printf("数据读取失败\n");
		return -1;
	}
	return 0;
}

void aht20_encode(unsigned char *databuf, int *ret_tem, int *ret_hum)
{
	int tem, hum;
	hum = databuf[1];
	hum = (hum<<8) | databuf[2];
	hum = (hum<<8) | databuf[3];
	hum >>= 4;
	hum = (hum*1000) >> 20;

	tem = databuf[3];
	tem &= 0x0000000F;
	tem = (tem<<8) | databuf[4];
	tem = (tem<<8) | databuf[5];
	tem = ((tem*2000) >> 20) - 500;

	*ret_hum = hum;
	*ret_tem = tem;
}

int read_aht20(int *ret_tem, int *ret_hum)
{
	unsigned char ori[AHT20_ORI_SIZE];
	int ret, hum, tem;
	
	if (read_aht20_origin(ori) < 0)
	{
		return -1;
	}

	aht20_encode(ori, ret_tem, ret_hum);
	
	return 0;
}

