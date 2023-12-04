#ifndef __KEY_CODE_H
#define __KEY_CODE_H

#include <linux/input.h>

struct key_table_t {
	unsigned char key_val;
	unsigned char key_code;
	unsigned char key_name[20];
};

#define KEY_NUM sizeof(key_table) / sizeof(key_table[0])

extern int open_ir(void);
extern int close_ir(void);
extern int ir_read(struct input_event *iebuf);
extern int ir_encode(struct input_event *ie, struct key_table_t *ktbuf);

#endif
