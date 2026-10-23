/*
 * button.c
 *
 * Created: 2026-06-10 오후 1:25:20
 *  Author: user
 */ 

#include "button.h"

void init_button(void);
uint8_t get_button(int button_num, int button_pin);
extern volatile uint32_t ms_counter;

// 버튼 초기화 방향설정(입력) 
void init_button(void)
{
	BUTTON_DDR &= ~(1 << BUTTON0PIN | 1 << BUTTON1PIN | 1 << BUTTON2PIN | 1 << BUTTON3PIN | 1 << BUTTON4PIN);
	// BUTTON_DDR &= 0xf0;
	// BUTTON_DDR &= 0x0f
}

// 예) BUTTON0 3
//    button을 눌렀다 떼면 : 1을 리턴
//          이 idle 상태 : 0을 리턴 

uint8_t get_button (int button_num, int button_pin)
{
	static int prev_st[5] = {0,0,0,0,0};
	int curr_st;
	
	curr_st = BUTTON_PIN & ( 1 << button_pin) ? 1 : 0;
	
	if(curr_st ==1 && prev_st[button_num] == 0)
	{
		_delay_ms(20);
		
		OCR3A = 399;
		_delay_ms(100);
		OCR3A = 299;
		_delay_ms(80);
		
		prev_st[button_num]=1;
		return 1;
	}
	else if (curr_st == 0 && prev_st[button_num]==1)
	{
		_delay_ms(20);
		
		OCR3A = 0;
		_delay_ms(100);

		prev_st[button_num]=0;
		return 0;
	}
	return 0;
}

