// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#ifndef __RC522_H__
#define __RC522_H__

#include <stddef.h>

#define RC522_PASSWD_SIZE	6
#define RC522_ID_SIZE		4
#define RC522_BLOCK_SIZE	16

#define RC522_MAGIC		'r'
#define RC522_CHANGE_PASSWD	_IOW(RC522_MAGIC, 1, unsigned char [RC522_PASSWD_SIZE])
#define RC522_CHANGE_BLOCK	_IOW(RC522_MAGIC, 2, unsigned char)
#define RC522_READ_CARD		_IO(RC522_MAGIC, 3)  
#define RC522_WRITE_CARD	_IO(RC522_MAGIC, 4)
#define RC522_CHANGE_KEY	_IO(RC522_MAGIC, 5)
#define RC522_GET_ID		_IOR(RC522_MAGIC, 6, char [RC522_ID_SIZE])

extern int open_rc522(void);
extern int close_rc522(void);
extern int rc522_ioctl(unsigned int cmd, unsigned long arg);
extern int rc522_getid(char *id);
extern int rc522_change_passwd(unsigned char *passwd);
extern int rc522_change_block(unsigned char block);
extern int rc522_write(char *writebuf, size_t cnt);
extern int rc522_read(char *readbuf);
#endif

