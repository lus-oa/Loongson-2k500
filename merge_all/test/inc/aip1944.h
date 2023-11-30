#ifndef __AIP1944_H__
#define __AIP1944_H__

#define AIP1944_SLIDE_MODE 1
#define AIP1944_ROLL_MODE 2

extern unsigned char aip1944_demo[384];

extern int open_aip1944(void);
extern int close_aip1944(void);
extern void aip1944_display(unsigned char *buf, int byte_cnt, int mode);
extern void aip1944_display_clear(void);
void _display_slide_(unsigned char *data, int byte_cnt);
void _display_roll_(unsigned char *data, int byte_cnt);

#endif