/*
 * 05.ULTRASONIC.c
 *
 * Created: 2026-06-15 오전 11:36:36
 * Author : kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> //인터럽트 관련 함수
#include <stdio.h>
#include "button.h"

// 초기화
extern void init_uart0(void);
extern void init_i2c(void);
extern void init_lcd(void);
extern void init_button(void);
extern void init_keypad (void);
extern uint8_t lcd_cal (void);
extern void UART0_transmit(void);
extern void i2c_clock(int *mode,int *time, char* line);
extern uint8_t get_button(int button_num, int button_pin);
extern void lcd_clear(void);
extern void i2c_calender(int *mode, int *time, char *line);
extern void change_clock_mode(int *mode, int *time, char *temp);
extern void change_calender_mode(int *mode, int *time, char *change_buff);
extern void init_speaker(void);
extern void keypad_sound (uint8_t key_data);
extern uint8_t scan_keypad(void);
void init_timer0(void);
volatile int ms_counter = 0;
volatile uint32_t keypad_counter=0;
volatile uint8_t key_data = 0;
volatile int wt_count = 0;
volatile int sec_count = 0;
volatile int mode = 0;

extern uint8_t key_event;
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

void (*fp_mode[])(int *, int *, char *) = {
	i2c_clock,
	i2c_calender,
	change_clock_mode,
	change_calender_mode,
};
ISR(TIMER0_OVF_vect)
{
	uint8_t temp;

	TCNT0 = 6;
	ms_counter++;

	if (mode == 4) {
		wt_count++;

		if (wt_count >= 1000) {
			wt_count = 0;
			sec_count++;
		}
	}

	/* 20ms마다 키패드 검사 */
	if (++keypad_counter >= 20) {
		keypad_counter = 0;

		temp = scan_keypad();
		key_data = temp;

		/* 설정 화면 입력은 처리할 때까지 보관 */
		if ((mode == 2 || mode == 3) &&
		temp != 0 &&
		key_event == 0) {

			key_event = temp;
		}

		/* 기존 코드는 '0'이라서 잘못된 비교였음 */
		if (temp != 0) {
			keypad_sound(temp);
		}
	}
}

int main(void) {
	int time[7];
	char line[17];

	init_uart0();
	init_timer0();
	init_lcd();
	init_i2c();
	init_keypad();
	init_button();
	init_speaker();
	
	stdout = &OUTPUT;
	sei(); // 전역(대문) interrupt 허용
	
	
//	i2c_main();
	while (1) {
		
		if (mode < 4)
			fp_mode[mode](&mode, time, line);
		// 일반 모드에서만 화면 전환 버튼 사용
		if (mode < 2) {
			if (get_button(BUTTON3, BUTTON3PIN)) {
				lcd_clear();
				mode = (mode + 1) % 2;
			}
			if (get_button(BUTTON2, BUTTON2PIN)) {
				sprintf(line, "%02d:%02d:%02d", time[2], time[1], time[0]);

				lcd_clear();
				mode = 2;
			}
		}
		if (get_button(BUTTON1, BUTTON1PIN)) {
			lcd_clear();
			mode = 4;
		}
		
		if (mode == 4) {
			
			lcd_cal();
			if (get_button(BUTTON1, BUTTON1PIN)) {
				mode = 0;
			}
		}

	}
}

void init_timer0(void) {
	TCNT0 = 6; // TCNT0 6~256 : 250개 펄스 count
	
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00); // 초기화 (0분주)
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00; //64분주 
	TIMSK |= 1 << TOIE0; // TIMER0 Overflow INT

}
