// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

#ifndef __KEY16_H__
#define __KEY16_H__

#include <stdint.h>

#define KEY16(idx)	(uint16_t)(1 << idx)

extern int open_key16(void);
extern int close_key16(void);
extern uint16_t key_scan(void);

#endif
