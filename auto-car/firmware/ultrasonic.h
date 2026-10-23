#ifndef AUTO_CAR_ULTRASONIC_H
#define AUTO_CAR_ULTRASONIC_H

/*
 * ultrasonic.h
 *
 * Created: 2026-06-17 오후 1:32:26
 *  Author: kccistc
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

 /* 초음파 TRIG */
#define TRIG_LEFT      PA0
#define TRIG_CENTER    PA1
#define TRIG_RIGHT     PA2

/* 초음파 ECHO */
#define ECHO_LEFT      PE4    // INT4
#define ECHO_CENTER    PE5    // INT5
#define ECHO_RIGHT     PE6    // INT6

/* 모터 방향 */
#define IN1 PF0
#define IN2 PF1
#define IN3 PF2
#define IN4 PF3

#define SENSOR_NONE    0
#define SENSOR_LEFT    1
#define SENSOR_CENTER  2
#define SENSOR_RIGHT   3

#define TIMEOUT_COUNT  7500

extern volatile uint16_t left_distance;
extern volatile uint16_t center_distance;
extern volatile uint16_t right_distance;

void init_ultrasonic(void);
uint16_t measure_sensor(uint8_t sensor, uint8_t trig_pin);
void read_all_sensor(void);

#endif /* AUTO_CAR_ULTRASONIC_H */
