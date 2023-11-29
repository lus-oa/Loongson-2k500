#ifndef __RC522_H__
#define __RC522_H__

#define RC522_MAGIC	'r'
#define CHANGE_PASSWD	_IO(RC522_MAGIC, 1)
#define CHANGE_BLOCK	_IO(RC522_MAGIC, 2)
#define READ_CARD	_IO(RC522_MAGIC, 3)  
#define WRITE_CARD	_IO(RC522_MAGIC, 4)
#define CHANGE_KEY	_IO(RC522_MAGIC, 5)
#define GET_ID		_IO(RC522_MAGIC, 6)

#endif

