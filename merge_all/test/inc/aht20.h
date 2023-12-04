// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#ifndef __AHT20_H__
#define __AHT20_H__

#define AHT20_ORI_SIZE		7	//Byte

extern int open_aht20(void);
extern int close_aht20(void);
extern int read_aht20_origin(unsigned char *databuf);
extern void aht20_encode(unsigned char *databuf, int *ret_tem, int *ret_hum);
extern int read_aht20(int *ret_tem, int *ret_hum);

#endif

