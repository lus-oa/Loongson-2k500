#include "rc522.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

static char filename[] = "/dev/rfid_dev";
static int fd = -1;

int open_rc522(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_rc522(void)
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

int read_aht20(int *ret_tem, int *ret_hum)
{
	unsigned char ori[AHT20_ORI_SIZE];
	int ret, hum, tem;

	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return -1;
	}
	
	if (read_aht20_origin(ori) < 0)
	{
		return -1;
	}

	aht20_encode(ori, ret_tem, ret_hum);
	
	return 0;
}

int main(int argc, char** argv) 
{ 
	int rc522_fd; 
	int i, read_num, write_num; 
	char r[256];
	char a[16]; 
	rc522_fd = open("/dev/rfid_dev", O_RDWR); 
	printf("test: rc522_fd=%d\n", rc522_fd); 
	if(rc522_fd == -1) 
	{ 
		printf("test: Error Opening rc522\n"); 
		return(-1); 
	} 
	printf("test: wait begin\n"); 
	sleep(1); //wait 
	printf("test: wait done\n"); 

	ioctl(rc522_fd, GET_ID, &(a[0]));//参数3：选第0块 */
	printf("%d%d%d%d", a[0],a[1],a[2],a[3]); 
	while (1)
	{ 
		read_num = read(rc522_fd, r, 0); 
		printf("read_num=%d\n", read_num); 
		for (i = 0; i < read_num; i++)
		{
			printf("%x ", r[i]);
		}
		printf("\n");
		if(read_num > 0){ 
			write_num = write(rc522_fd, "\1\1\1", 3);
			printf("write_num=%d\n", write_num); 
			
			read_num = read(rc522_fd, r, 0); 
			printf("read again read_num=%d\n", read_num); 
			for (i = 0; i < read_num; i++)
			{
				printf("%x ", r[i]);
			}
			printf("\n");
			break;
		} 
		sleep(1); 
	}
	close(rc522_fd);
	return 0; 
} 

