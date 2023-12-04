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

int main(int argc, char *argv[])
{
	int fd;
	char *filename = "/dev/aht20";
	unsigned char readByte[6];//忽略CRC校验
	int ret = 0, i;
        unsigned int  H1=0;  //Humility
        unsigned int  T1=0; 
	fd = open(filename, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}
       
        while (1) {
		ret = read(fd, readByte, sizeof(readByte));
		if((readByte[0] & 0x68) == 0x08) {  /* 数据读取成功 */
		        H1 = readByte[1];
		        H1 = (H1<<8) | readByte[2];
		        H1 = (H1<<8) | readByte[3];
		        H1 = H1>>4;

		        H1 = (H1*1000) >> 20;

		        T1 = readByte[3];
		        T1 = T1 & 0x0000000F;
		        T1 = (T1<<8) | readByte[4];
		        T1 = (T1<<8) | readByte[5];

		        T1 = ((T1*2000) >> 20) - 500;
			printf("Temp:%d%d.%d\r\n",T1/100,(T1/10)%10,T1%10);
	                printf("Hum:%d%d.%d\r\n",H1/100,(H1/10)%10,H1%10);
		}
		else
		{
			printf("数据读取失败\n");
		}
		sleep(2);
	}

        close(fd);	/* 关闭文件 */	
	return 0;

}
