// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

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
#include <linux/input.h>
#include "../key_code.h"

//./keyinputApp /dev/input/event0

/* 定义一个input_event变量，存放输入事件信息 */
static struct input_event inputevent;

static unsigned int get_key_table_id(unsigned int code)
{
	int id;
	for (id = 0; id < KEY_NUM; id++) {
		if (code == key_table[id].key_code) {
			if (KEY_NUM >= id)
				return id;
		}
	}
	return 0xFFFF;
}

/*
 * @description		: main主程序
 * @param - argc 	: argv数组元素个数
 * @param - argv 	: 具体参数
 * @return 			: 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
	int fd, id = 0;
	int err = 0;
	char *filename;

	filename = argv[1];

	if (argc != 2) {
		printf("Error Usage!\r\n");
		return -1;
	}

	fd = open(filename, O_RDWR);
	if (fd < 0) {
		printf("Can't open file %s\r\n", filename);
		return -1;
	}

	while (1) {
		err = read(fd, &inputevent, sizeof(inputevent));
		if (err > 0) { /* 读取数据成功 */
			//printf("inputevent.type:%x,EV_KEY:%x,EV_REL:%x,EV_ABS:%x,EV_MSC:%x,EV_SW:%x,BTN_MISC:%x\r\n",inputevent.type,EV_KEY,EV_REL,EV_ABS,EV_MSC,EV_SW,BTN_MISC);
			//printf("key %d %s\r\n", inputevent.code, inputevent.value ? "press" : "release");
			id = get_key_table_id(inputevent.code);
			switch (inputevent.type) {
			case EV_KEY:
				if (inputevent.code < BTN_MISC) { /* 键盘键值 */
					printf("key %s %s\r\n",
					       key_table[id].key_name,
					       inputevent.value ? "press" :
								  "release");
				}
				break;

			/* 其他类型的事件，自行处理 */
			case EV_REL:
				break;
			case EV_ABS:
				break;
			case EV_MSC:
				break;
			case EV_SW:
				break;
			}
		} else {
			//printf("reading data failed\r\n");
		}
	}
	return 0;
}
