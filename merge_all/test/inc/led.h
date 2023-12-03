#ifndef __AHT20_H__
#define __AHT20_H__

#define LED_COUNT	3

extern int open_led(void);
extern int close_led(void);
extern void led_control(unsigned char *buf);
extern void led_on(int idx);
extern void led_off(int idx);

#endif

