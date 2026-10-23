/*
 * button.h
 *
 * Created: 2026-06-10 오전 11:45:01
 *  Author: user
 */ 

#define F_CPU 16000000UL  //16MHz
#include <avr/io.h>  // PORTA PORTB PORTD... IO관련 reg가 들어 있다. 
#include <util/delay.h>  // _delay_ms _delay_us 등 

#define  BUTTON_DDR  DDRD
#define  BUTTON_PIN  PIND   // PORTD를 읽는 register 5v:1  0v:0 

#define  BUTTON0PIN 3 
#define  BUTTON1PIN 4 
#define  BUTTON2PIN 5 
#define  BUTTON3PIN 6 
#define  BUTTON4PIN 7

#define  BUTTON0   0   // PORTD.3의 가상 index (sw번호)
#define  BUTTON1   1   // PORTD.4의 가상 index (sw번호)
#define  BUTTON2   2   // PORTD.5의 가상 index (sw번호)
#define  BUTTON3   3   // PORTD.6의 가상 index (sw번호)
#define	 BUTTON4   4
#define  BUTTON_NUMBER  5   // 버튼 갯수 

#define  BUTTON_PRESS  1      // 버튼을 누르면 high (active-high)
#define  BUTTON_RELEASE 0     // 버튼을 뗀 상태 (low)