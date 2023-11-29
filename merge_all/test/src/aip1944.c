#include "aip1944.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static char filename[] = "/dev/aip";
static int fd = -1;

int open_aip1944(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_aip1944(void)
{
	int ret;
	ret = close(fd);
	if (ret < 0) {
		printf("can't close file %s\n", filename);
		return -1;
	}
	fd = -1;
	return ret;
}

void display_sdu(void)
{
	int ret;

	if (fd < 0) {
		printf("没有打开文件：%s\n", filename);
		return;
	}
	int frames = sizeof(my_arr) / sizeof(u_int8_t) - 32;
	int begin = 0;
	while (frames >= 0) {
		ret = write(fd, my_arr + begin, 32 * sizeof(u_int8_t));
		usleep(1000 * 33);
		frames -= 2;
		begin += 2;
	}
}

void display_clear(void)
{
	int ret;

	if (fd < 0) {
		printf("没有打开文件：%s\n", filename);
		return;
	}
	unsigned char buf[32];
	memset(buf, 0x00, sizeof(buf) / sizeof(unsigned char));
	ret = write(fd, buf, 32 * sizeof(u_int8_t));
}