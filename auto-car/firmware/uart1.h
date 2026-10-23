#ifndef AUTO_CAR_UART1_H
#define AUTO_CAR_UART1_H

/*
 * uart1.h
 *
 * Created: 2026-06-22 오전 10:38:00
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL  // 16MHz
#include <avr/io.h>  // PORTA PORTB PORTD... IO관련 reg가 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us 등
#include <avr/interrupt.h>
#include <string.h>

extern volatile uint8_t bt_data;

void init_uart1(void);
void UART1_transmit(uint8_t data);

#endif /* AUTO_CAR_UART1_H */
