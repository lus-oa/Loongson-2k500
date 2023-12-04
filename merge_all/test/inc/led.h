// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#ifndef __LED_H__
#define __LED_H__

#define LED_COUNT	3

extern int open_led(void);
extern int close_led(void);
extern void led_control(unsigned char *buf);
extern void led_on(int idx);
extern void led_off(int idx);
extern void led_neg(int idx);

#endif

