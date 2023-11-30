#include "rc522.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

static char filename[] = "/dev/rfid_dev";
static int fd = -1;

int open_rc522(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_rc522(void)
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

int rc522_ioctl(unsigned int cmd, unsigned long arg)
{
	int ret;
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return -1;
	}

	if (_IOC_DIR(cmd) & (_IOC_READ | _IOC_WRITE))
	{
		ret = ioctl(fd, cmd, arg);
	}
	else
	{
		ret = ioctl(fd, cmd);
	}
	
	if (ret < 0)
	{
		printf("智能卡操作失败\n");
		return -1;
	}

	return 0;
}

int rc522_getid(char *id)
{
	return rc522_ioctl(RC522_GET_ID, (unsigned long)id);
}

int rc522_change_passwd(unsigned char *passwd)
{
	return rc522_ioctl(RC522_CHANGE_PASSWD, (unsigned long)passwd);
}

int rc522_change_block(unsigned char block)
{
	return rc522_ioctl(RC522_CHANGE_BLOCK, block);
}

int rc522_read(char *readbuf)
{
	int ret;
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return 0;
	}
	
	ret = read(fd, readbuf, RC522_BLOCK_SIZE);
	if (ret < RC522_BLOCK_SIZE)
	{
		printf("读取块失败\n");
		return 0;
	}
	
	return ret;
}

int rc522_write(char *writebuf, size_t cnt)
{
	int ret;
	if (fd < 0)
	{
		printf("没有打开文件:%s\n", filename);
		return 0;
	}
	
	ret = write(fd, writebuf, cnt);
	if (ret < cnt)
	{
		printf("写块失败\n");
		return 0;
	}
	
	return ret;
}

