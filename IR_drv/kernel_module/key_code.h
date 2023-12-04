// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

#ifndef __KEY_CODE_H
#define __KEY_CODE_H

struct key_table_t {
	unsigned char key_val;
	unsigned char key_code;
	unsigned char key_name[20];
};

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

#define KEY_NUM sizeof(key_table) / sizeof(key_table[0])

#endif
