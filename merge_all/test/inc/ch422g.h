#ifndef __CH422G_H__
#define __CH422G_H__

#define CH422G_M1	0x01	//中上
#define CH422G_R1	0x02	//右上
#define CH422G_R2	0x04	//右下
#define CH422G_M3	0x08	//中下
#define CH422G_L2	0x10	//左下
#define CH422G_L1	0x20	//左上
#define CH422G_M2	0x40	//中间
#define CH422G_PT	0x80	//点

extern int open_ch422g(void);
extern int close_ch422g(void);
extern void ch422g_flush(void);
extern void ch422g_set_char(int idx, char ch);
extern void ch422g_set_char_flush(int idx, char ch);
extern void ch422g_set_num(int idx, int num);
extern void ch422g_set_num_flush(int idx, int num);
extern void ch422g_set_tube(int idx, unsigned char ch);
extern void ch422g_set_tube_flush(int idx, unsigned char ch);
extern void ch422g_set_mask(int idx, unsigned char mask);
extern void ch422g_set_mask_flush(int idx, unsigned char mask);
extern void ch422g_clear_mask(int idx, unsigned char mask);
extern void ch422g_clear_mask_flush(int idx, unsigned char mask);
extern void ch422g_reset(void);
extern const unsigned char BCD_decode_tab[];

#endif

