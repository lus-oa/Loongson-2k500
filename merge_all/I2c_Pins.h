// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#ifndef __I2C_PINS__
#define __I2C_PINS__

#define SDA3_GPIO		68
#define SCL3_GPIO		69
#define CH422G_SDA_GPIO		142
#define CH422G_SCL_GPIO		141

typedef struct softi2c_pins {
	int sda;
	int scl;
} I2c_Pins;

#endif

