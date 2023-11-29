#ifndef __MOTOR_H__
#define __MOTOR_H__

#define MOTOR_MAGIC	'k'
#define CMD_standby	_IO(MOTOR_MAGIC, 1)
#define CMD_forward	_IO(MOTOR_MAGIC, 2)
#define CMD_backward	_IO(MOTOR_MAGIC, 3)
#define CMD_brake	_IO(MOTOR_MAGIC, 4)

extern int open_motor(void);
extern int close_motor(void);
extern int motor_control(unsigned int cmd);
extern void motor_standby(void);
extern void motor_forward(void);
extern void motor_backward(void);
extern void motor_brake(void);

#endif

