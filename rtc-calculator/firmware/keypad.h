/*
 * keypad.h
 *
 * Created: 2026-07-01 오전 9:48:53
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL  
#include <avr/io.h>  
#include <util/delay.h>  
#include <avr/interrupt.h>

#define KEYPAD_DDR DDRF
#define KEYPAD_PORT PORTF
#define KEYPAD_PIN PINF

void init_keypad (void);
uint8_t get_keypad ( int row, int col);
uint8_t scan_keypad (void);

void init_button (void);
uint8_t get_button (int button_num, int button_pin);

