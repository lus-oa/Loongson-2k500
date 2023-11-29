#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define BEEP_MAGIC	'b'
#define BEEP_OFF 	_IO(BEEP_MAGIC, 1)	/* 关蜂鸣器 */
#define BEEP_ON 	_IO(BEEP_MAGIC, 2)	/* 开蜂鸣器 */
void beep_discont(int fd, int count, unsigned long ms);
void beep_on(int fd);
void beep_off(int fd);

int main(int argc, char *argv[])
{
	int fd, ret;

	/* 打开beep驱动 */
	fd = open("/dev/beep", O_RDWR);
	if(fd < 0){
		printf("Device beep open failed!\r\n");
		return -1;
	}
	
	beep_discont(fd, 2, 100);

	ret = close(fd); /* 关闭文件 */
	if(ret < 0){
		printf("file %s close failed!\r\n", "/dev/beep");
		return -1;
	}

	return 0;
}

void beep_discont(int fd, int count, unsigned long ms)
{
	for (int i = 0; i < count; i++)
	{
		beep_on(fd);
		usleep(1000 * ms);
		beep_off(fd);
		usleep(1000 * ms);
	}
}

void beep_on(int fd)
{
	ioctl(fd, BEEP_ON);
}

void beep_off(int fd)
{
	ioctl(fd, BEEP_OFF);
}

