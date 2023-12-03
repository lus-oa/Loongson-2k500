#ifndef __AHT20_H__
#define __AHT20_H__

#define LED_COUNT	3

extern int open_led(void);
extern int close_led(void);
extern int led_flush(void);
extern void led_control(unsigned char *buf);
extern void led1_on(void);
extern void led1_off(void);
extern void led2_on(void);
extern void led2_off(void);
extern void led3_on(void);
extern void led3_off(void);

#endif

