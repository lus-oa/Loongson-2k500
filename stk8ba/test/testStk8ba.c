// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

short axis_out_encode(unsigned char out1, unsigned char out2)
{
	short val;
	val = (out1 >> 4) | (out2 << 4); 
	if (val & (1 << 11))
	{
		val |= 0xF000;
	}
	return val;
}

int main(int argc, char *argv[])
{
	int fd;
	char *filename = "/dev/stk8ba";
	unsigned char databuf[6];
	int ret = 0;
        unsigned char xout1,xout2,yout1,yout2,zout1,zout2;
	short x, y, z;
	int t = 20;

	fd = open(filename, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}
       
        while (t--) {
		ret = read(fd, databuf, sizeof(databuf));
		if(ret == 0) { 			/* 数据读取成功 */
		        xout1 = databuf[0];
			xout2 = databuf[1];
			yout1 = databuf[2];
                        yout2 = databuf[3];
                        zout1 = databuf[4];
			zout2 = databuf[5];
			x = axis_out_encode(xout1, xout2);
			y = axis_out_encode(yout1, yout2);
			z = axis_out_encode(zout1, zout2);
		}
			printf("x:%d, y:%d, z:%d\n", x, y, z);
		usleep(500000); /*500ms */
	}

        close(fd);	/* 关闭文件 */	
	return 0;
}

