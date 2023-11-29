#include "stk8ba.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


static char filename[] = "/dev/stk8ba";
static int fd = -1;

int open_stk8ba(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_stk8ba(void)
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

static short axis_out_encode(unsigned char out1, unsigned char out2)
{
	short val;
	val = (out1 >> 4) | (out2 << 4); 
	if (val & (1 << 11))
	{
		val |= 0xF000;
	}
	return val;
}

int read_stk8ba_origin(unsigned char *databuf)
{
	int ret;
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return -1;
	}

	ret = read(fd, databuf, STK8BA_ORI_SIZE);

	if (ret != STK8BA_ORI_SIZE)
	{
		printf("读取数据失败\n");
		return -1;
	}

	return 0;
}

int read_stk8ba_xyz(int *databuf)
{
	unsigned char ori[STK8BA_ORI_SIZE];
	int i;
	
	if (read_stk8ba_origin(ori) < 0)
	{
		return -1;
	}

	for (i = 0; i < STK8BA_XYZ_SIZE; i++)
	{
		databuf[i] = axis_out_encode(ori[i*2], ori[i*2+1]);
	}
	
	return 0;
}

