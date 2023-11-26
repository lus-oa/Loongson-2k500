#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

/* 定义按键值 */
#define KEY0VALUE 0XF0
#define INVAKEY 0X00

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int has_press = 0;

	int fd, ret;
	char *filename;
	int keyvalue[8];
	unsigned char databuf[4];

	if (argc < 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	/* 打开key驱动 */
	fd = open("/dev/key", O_RDWR);
	if (fd < 0) {
		printf("file /dev/key open failed!\r\n");
		return -1;
	}

	strcpy(databuf, argv[1]);
	for (int i = 0; i < 4; i++) {
		databuf[i] -= '0';
	}
	ret = write(fd, databuf, sizeof(databuf));
	if (ret < 0) {
		printf("Key Control Failed!\r\n");
		close(fd);
		return -1;
	}

	/* 循环读取按键值数据！ */
	while (1) {
		read(fd, keyvalue, sizeof(keyvalue));
		printf("KEY0 Press, value = %#X-%#X-%#X-%#X-%#X-%#X-%#X-%#X\r\n",
		       keyvalue[0], keyvalue[1], keyvalue[2], keyvalue[3],
		       keyvalue[4], keyvalue[5], keyvalue[6], keyvalue[7]);
		sleep(1);
	}

	ret = close(fd); /* 关闭文件 */
	if (ret < 0) {
		printf("file %s close failed!\r\n", argv[1]);
		return -1;
	}
	return 0;
}

//./test_key 1000
//./test_key 0100
//./test_key 0010
//./test_key 0001
