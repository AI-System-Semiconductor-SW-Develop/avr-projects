#ifndef AUTO_CAR_PWM_H
#define AUTO_CAR_PWM_H

/*
 * pwm.h
 *
 * Created: 2026-06-18 오후 2:22:57
 *  Author: kccistc
 */ 

#include "button.h"
#include <avr/interrupt.h>
#include <stdio.h>
#define MOTOR_PWM_DDR  DDRB
#define MOTOR_LEFT_PORT_DDR  5    // OC1A
#define MOTOR_RIGHT_PORT_DDR  6   // OC1B

#define MOTOR_DIRECTION_PORT  PORTF
#define MOTOR_DIRECTION_PORT_DDR  DDRF

#define MOTOR_LEFT_PWM_PIN   MOTOR_LEFT_PORT_DDR
#define MOTOR_RIGHT_PWM_PIN  MOTOR_RIGHT_PORT_DDR
#define MOTOR_DIRECTION_DDR  MOTOR_DIRECTION_PORT_DDR
#define MOTOR_DIR_MASK       0x0f

void init_timer1_pwm(void);
void init_motor_driver(void);
void init_motor(void);
void forward(int speed);
void backward(int speed);
void turn_left(int speed);
void turn_right(int speed);
void stop(void);

#endif /* AUTO_CAR_PWM_H */
