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
	char *filename = "/dev/adc";
	unsigned char readByte[3],cmd[1];
	int ret = 0;
        short val1,val2;
        unsigned char *p1,*p2;
        float adc0,adc1;

        p1 = (unsigned char*)&val1;
        p2 = (unsigned char*)&val2;

	fd = open(filename, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}
         
        while (1) {
                cmd[0] = 0xdc;
                ret = write(fd, cmd, sizeof(cmd));
		ret = read(fd, readByte, sizeof(readByte));
                p1[0] = readByte[1];
                p1[1] = readByte[0];
                adc0 = ((val1+55.0)/(32767.0+55.0))*2.048;
                //printf("ain0 readByte:%x,%x,%x,%d,%.2f\n",readByte[0],readByte[1],readByte[2],val1,adc0);
                printf("ain0 %d,%.2f\n",val1,adc0);
		usleep(1000*1000);

                cmd[0] = 0xfc;
                ret = write(fd, cmd, sizeof(cmd));
		ret = read(fd, readByte, sizeof(readByte));
                p2[0] = readByte[1];
                p2[1] = readByte[0];
                adc1 = ((val2+55.0)/(32767.0+55.0))*2.048;
                //printf("ain1 readByte:%x,%x,%x,%d,%.2f\n",readByte[0],readByte[1],readByte[2],val2,adc1);
                printf("ain1 %d,%.2f\n",val2,adc1);
		usleep(1000*1000);
	}

        close(fd);	/* 关闭文件 */	
	return 0;
}
