// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include "ir.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <poll.h>
#include <signal.h>

static char filename[] = "/dev/input/event2";
static int fd = -1;
static struct input_event inputevent;
struct key_table_t key_table[] = {
	{ 0x40, KEY_UP, "UP" },	      { 0x19, KEY_DOWN, "DOWN" },
	{ 0x7, KEY_LEFT, "LEFT" },    { 0x9, KEY_RIGHT, "RIGHT" },
	{ 0x15, KEY_ENTER, "ENTER" }, { 0xc, KEY_1, "Num:1" },
	{ 0x18, KEY_2, "Num:2" },     { 0x5e, KEY_3, "Num:3" },
	{ 0x8, KEY_4, "Num:4" },      { 0x1c, KEY_5, "Num:5" },
	{ 0x5a, KEY_6, "Num:6" },     { 0x42, KEY_7, "Num:7" },
	{ 0x52, KEY_8, "Num:8" },     { 0x4a, KEY_9, "Num:9" },
	{ 0x16, KEY_0, "Num:0" },     { 0x45, KEY_A, "A" },
	{ 0x46, KEY_B, "B" },	      { 0x47, KEY_C, "C" },
	{ 0x44, KEY_D, "D" },	      { 0x43, KEY_E, "E" },
	{ 0xd, KEY_F, "F" },
};

int open_ir(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_ir(void)
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

int ir_encode(struct input_event *ie, struct key_table_t *ktbuf)
{
	int id;
	for (id = 0; id < KEY_NUM; id++) {
		if (ie->code == key_table[id].key_code)
		{
			memcpy(ktbuf, &key_table[id], sizeof(struct key_table_t));
			return 0;
		}
	}
	return -1;
}

int ir_read(struct input_event *iebuf)
{
	if (fd < 0)
	{
		printf("红外遥控未启用\n");
		return -1;
	}
	if (!read(fd, iebuf, sizeof(struct input_event)))
	{
		printf("红外数据读取失败\n");
		return -1;
	}
	return 0;
}

