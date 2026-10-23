#ifndef AUTO_CAR_BUTTON_H
#define AUTO_CAR_BUTTON_H

/*
 * button.h
 *
 * Created: 2026-06-10 오전 11:45:01
 *  Author: user
 */ 

#define F_CPU 16000000UL  //16MHz
#include <avr/io.h>  // PORTA PORTB PORTD... IO관련 reg가 들어 있다. 
#include <util/delay.h>  // _delay_ms _delay_us 등 

#define  BUTTON_DDR  DDRB
#define  BUTTON_PIN  PINB   // PORTD를 읽는 register 5v:1  0v:0 
#define BUTTON_PORT PORTB

#define  BUTTON0PIN 4    // PORTB.3

#define  BUTTON0   0   // PORTB.3의 가상 index (sw번호)

#define  BUTTON_PRESS  1      // 버튼을 누르면 high (active-high)
#define  BUTTON_RELEASE 0     // 버튼을 뗀 상태 (low)
#define BUTTON_NUMBER 1

void init_button(void);
int get_button(int button_num, int button_pin);

#endif /* AUTO_CAR_BUTTON_H */
