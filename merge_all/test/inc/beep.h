#ifndef __BEEP_H__
#define __BEEP_H__

#define BEEP_MAGIC	'b'
#define BEEP_OFF 	_IO(BEEP_MAGIC, 1)	/* 关蜂鸣器 */
#define BEEP_ON 	_IO(BEEP_MAGIC, 2)	/* 开蜂鸣器 */

extern int open_beep(void);
extern int close_beep(void);
extern void beep_on(void);
extern void beep_off(void);

#endif

