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
#include <math.h>

#define PI 3.14159265

short My_Sin(short deg) 
{
    return 100*sin(deg);
}

int main(int argc, char *argv[])
{
	int fd;
	char *filename;
	unsigned char databuf[17];
	int ret = 0,flag = 0;
        short tmp = 0x1122,deg = 0;
        unsigned char *p = (unsigned char*)&tmp;
        double val;

        val = PI / 180;

        //printf("======p0:%x,p1:%x\r\n",p[0],p[1]);

	fd = open("/dev/dac", O_RDWR);
	if(fd < 0) {
		printf("can't open file /dev/dac\r\n");
		return -1;
	}

        databuf[0] = 11;
   
while(1) {
        tmp = My_Sin(deg);
        //tmp = 100*sin(deg*val);
        databuf[1] = p[0];
        databuf[2] = p[1];
        
        deg += 10;
        if(deg>360)
           deg = 0;
        printf("tmp:%d,databuf[1]:%x,databuf[2]:%x\n",tmp,databuf[1],databuf[2]);

        ret = write(fd, databuf, 3);
	if(ret < 0){
		printf("dac Control Failed!\r\n");
		close(fd);
		return -1;
	}

        usleep(10);
}
       
    
/*
while(1) {       
        if(0==flag) {
          // databuf[1] = 1;
           //databuf[2] = 1;
           databuf[3] = 1;
           databuf[4] = 1;
           databuf[5] = 1;
           databuf[6] = 1;
           databuf[7] = 1;
           databuf[8] = 1;
           databuf[9] = 1;
           databuf[10] = 1;
           databuf[11] = 1;
           databuf[12] = 1;
           databuf[13] = 1;
           databuf[14] = 1;
           databuf[15] = 1;
           databuf[16] = 1;
        } 
        else {
           //databuf[1] = 0;
          // databuf[2] = 0;
           databuf[3] = 0;
           databuf[4] = 0;
           databuf[5] = 0;
           databuf[6] = 0;
           databuf[7] = 0;
           databuf[8] = 0;
           databuf[9] = 0;
           databuf[10] = 0;
           databuf[11] = 0;
           databuf[12] = 0;
           databuf[13] = 0;
           databuf[14] = 0;
           databuf[15] = 0;
           databuf[16] = 0;
        }
        flag = !flag;
        
        ret = write(fd, databuf, sizeof(databuf));
	if(ret < 0){
		printf("dac Control Failed!\r\n");
		close(fd);
		return -1;
	}

        usleep(10);
}
*/
        close(fd);	/* 关闭文件 */	
	return 0;

}
