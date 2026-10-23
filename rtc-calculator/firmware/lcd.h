/*
 * lcd.h
 *
 * Created: 2026-07-01 오전 9:44:52
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define LCD_DATA_DDR DDRC
#define LCD_DATA_PORT PORTC
#define LCD_CONTROL_DDR DDRA
#define LCD_CONTROL_PORT PORTA
#define LCD_RS_PIN 0
#define LCD_RW_PIN 1
#define LCD_E_PIN 2

#define LCD_DISPLAY_ON_OFF_BIT 2
#define LCD_CURSOR_BIT 1
#define LCD_BLINK_BIT 0