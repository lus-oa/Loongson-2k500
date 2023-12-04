// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd, retvalue;
	char *filename;
	unsigned char databuf[3];

	if (argc != 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	/* 打开led驱动 */
	fd = open("/dev/led", O_RDWR);
	if (fd < 0) {
		printf("file /dev/led open failed!\r\n");
		return -1;
	}

	databuf[0] = argv[1][0] - '0'; /* 要执行的操作：打开或关闭 */
	databuf[1] = argv[1][1] - '0'; /* 要执行的操作：打开或关闭 */
	databuf[2] = argv[1][2] - '0'; /* 要执行的操作：打开或关闭 */
	printf("databuf[0]:%d,databuf[1]:%d,databuf[2]:%d\r\n", databuf[0],
	       databuf[1], databuf[2]);

	/* 向/dev/led文件写入数据 */
	retvalue = write(fd, databuf, sizeof(databuf));
	if (retvalue < 0) {
		printf("LED Control Failed!\r\n");
		close(fd);
		return -1;
	}

	retvalue = close(fd); /* 关闭文件 */
	if (retvalue < 0) {
		printf("file %s close failed!\r\n", "/dev/led");
		return -1;
	}
	return 0;
}
