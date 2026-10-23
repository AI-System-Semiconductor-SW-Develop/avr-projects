/*
 * uart0.h
 *
 * Created: 2026-06-16 오전 9:57:31
 *  Author: kccistc
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define QUEUE_SIZE 10
#define QUEUE_LENGTH 80
volatile int u_rear = 0;
volatile int u_front = 0;

volatile unsigned char rx_buff[QUEUE_SIZE][QUEUE_LENGTH];