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

struct my_tm {
	int sec, min, hour;
	int week;
	int day, mon, year;
};

int sd2068_settime(int fd, struct my_tm *time_data);
int sd2068_gettime(int fd, struct my_tm *time_data);
void show_my_tm(struct my_tm *time_data);
unsigned char NUM_TO_BCD(int num);
int BCD_TO_NUM(unsigned char bcd);

int main(int argc, char *argv[])
{
	int fd;
	int ret = 0;
        struct my_tm time_data;
	char *filename = "/dev/sd2068";

	fd = open(filename, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}

        if(argc == 4 && *argv[1]=='w') {
          	sscanf(argv[2], "%d-%d-%d", &time_data.year, &time_data.mon, &time_data.day);
          	sscanf(argv[3], "%d:%d:%d", &time_data.hour, &time_data.min,&time_data.sec);
		printf("time setting:\n");
		ret = sd2068_settime(fd, &time_data);
		if (ret < 0)
		{
			printf("Error: Time Setting\n");
			close(fd);
			return -1;
		}
		show_my_tm(&time_data);
		printf("Time set done\n");
        }
        
        while (1) {
		ret = sd2068_gettime(fd, &time_data);
		if(ret == 0) { 			/* 数据读取成功 */
		       	show_my_tm(&time_data);
		}
		else
		{
			printf("Error: Time Reading\n");
		}
		sleep(1); /*100ms */
	}

        close(fd);	/* 关闭文件 */	
	return 0;
}

unsigned char NUM_TO_BCD(int num)
{
	return (((num / 10) << 4) | (num % 10));
}

int BCD_TO_NUM(unsigned char bcd)
{
	return ((bcd >> 4) * 10 + (bcd & 0x0F));
}

int sd2068_settime(int fd, struct my_tm *time_data)
{
	unsigned char write_buf[7];
	write_buf[0] = NUM_TO_BCD(time_data->sec);
	write_buf[1] = NUM_TO_BCD(time_data->min);
	write_buf[2] = NUM_TO_BCD(time_data->hour);
	write_buf[4] = NUM_TO_BCD(time_data->day);
	write_buf[5] = NUM_TO_BCD(time_data->mon);
	write_buf[6] = NUM_TO_BCD(time_data->year);

	//只编写24小时制
	write_buf[2] |= 0x80;

	return write(fd, write_buf, sizeof(write_buf));
}

int sd2068_gettime(int fd, struct my_tm *time_data)
{
	unsigned char read_buf[7];
	int ret;
	
	ret = read(fd, read_buf, sizeof(read_buf));
	
	time_data->sec = BCD_TO_NUM(read_buf[0]);
	time_data->min = BCD_TO_NUM(read_buf[1]);
	time_data->hour = BCD_TO_NUM(read_buf[2] & (~0x80));
	time_data->day = BCD_TO_NUM(read_buf[4]);
	time_data->mon = BCD_TO_NUM(read_buf[5]);
	time_data->year = BCD_TO_NUM(read_buf[6]);

	return ret;
}

void show_my_tm(struct my_tm *time_data)
{
	printf("date time is 20%d-%d-%d %d:%d:%d\n", time_data->year, time_data->mon, time_data->day, time_data->hour, time_data->min, time_data->sec);
}

