#include "ch422g.h"

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/ioctl.h> 

static char filename[] = "/dev/ch422g";
static int fd = -1;
static unsigned char tubes[5];
const unsigned char BCD_decode_tab[ 29 ] = { 0X3F, 0X06, 0X5B, 0X4F, 0X66, 0X6D, 0X7D, 0X07, 0X7F, 0X6F, 0X77, 0X7C, 0X58, 0X5E, 0X79, 0X71, 0x00, 0x46, 0x40, 0x41, 0x39, 0x0F, 0x08, 0x76, 0x38, 0x73, 0x80, 0xFF, 0x00 };

int open_ch422g(void)
{
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		printf("can't open file %s\n", filename);
		return -1;
	}
	return fd;
}

int close_ch422g(void)
{
	int ret;
	ch422g_reset();
	ret = close(fd);
	if (ret < 0)
	{
		printf("can't close file %s\n", filename);
		return -1;
	}
	fd = -1;
	return ret;
}

static void __ch422g_write(void)
{
	if (fd < 0)
	{
		printf("没有打开文件：%s\n", filename);
		return;
	}
	
	if (write(fd, tubes, sizeof(tubes)) < 0)
	{
		printf("数码管写入失败\n");
	}
}

static void __ch422g_set_char(int idx, char ch)
{
	if (ch <= '9' && ch >= '0')
	{
		tubes[idx] = BCD_decode_tab[ch - '0'];
	}
	else if (ch <= 'F' && ch >= 'A')
	{
		tubes[idx] = BCD_decode_tab[10 + ch - 'A'];
	}
	else if (ch <= 'f' && ch >= 'a')
	{
		tubes[idx] = BCD_decode_tab[10 + ch - 'a'];
	}
	else if (ch == ' ')
	{
		tubes[idx] = '\0';
	}
	else
	{
		tubes[idx] = ch;
	}
}

void ch422g_set_char(int idx, char ch)
{
	__ch422g_set_char(idx, ch);
	__ch422g_write();
}

void ch422g_set_tube(int idx, unsigned char ch)
{
	tubes[idx] = ch;
	__ch422g_write();
}

void ch422g_set_char_all(char *chs)
{
	int i;
	for (i = 1; i <= 4; i++)
	{
		__ch422g_set_char(i, chs[i]);
	}
	__ch422g_write();
}

void ch422g_set_tube_all(unsigned char *chs)
{
	int i;
	for (i = 1; i <= 4; i++)
	{
		tubes[i] = chs[i];
	}
	__ch422g_write();
}

void ch422g_reset(void)
{
	ch422g_set_char_all(" \0\0\0\0");
}

