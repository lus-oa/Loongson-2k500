#include "key16.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static char filename[] = "/dev/key";
static int fd = -1;
int keyvalue[8];

int open_key16(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_key16(void)
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

uint16_t key_scan(void)
{
	if (fd < 0) {
		printf("没有打开文件：%s\n", filename);
		return (uint16_t)-1;
	}
	int ret;
	uint16_t key = 0x00;
	unsigned char databuf[4];
	memset(databuf, 0x1, sizeof(databuf));
	memset(keyvalue, 0x0, sizeof(keyvalue));
	ret = write(fd, databuf, sizeof(databuf));
	if (ret < 0) {
		printf("Key Control Failed!\r\n");
		close(fd);
		return (uint16_t)-1;
	}
	while (1) {
		read(fd, keyvalue, sizeof(keyvalue));
		int flag = 0;
		for (int i = 0; i < 4; i++) {
			if (keyvalue[i] == 1) {
				flag = 1;
			}
		}
		if (flag)
			break;
		usleep(1000 * 20);
	}
	for (int i = 0; i < 4; i++) {
		memset(databuf, 0x0, sizeof(databuf));
		databuf[i] = 1;
		ret = write(fd, databuf, sizeof(databuf));
		if (ret < 0) {
			printf("Key Control Failed!\r\n");
			close(fd);
			return (uint16_t)-1;
		}
		usleep(1000 * 20);
		read(fd, keyvalue, sizeof(keyvalue));
		printf("KEY0 Press, value = %#X-%#X-%#X-%#X-%#X-%#X-%#X-%#X\r\n",
		       keyvalue[0], keyvalue[1], keyvalue[2], keyvalue[3],
		       keyvalue[4], keyvalue[5], keyvalue[6], keyvalue[7]);
		for (int j = 0; j < 4; j++) {
			if (keyvalue[j]) {
				key = key | (1 << i + j * 4);
			}
		}
	}
	return key;
}