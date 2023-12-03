#ifndef __KEY16_H__
#define __KEY16_H__

#include <stdint.h>

extern int open_key16(void);
extern int close_key16(void);
extern uint16_t key_scan(void);

#endif
