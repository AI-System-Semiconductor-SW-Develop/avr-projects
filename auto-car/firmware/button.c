/*
 * button.c
 *
 * Created: 2026-06-10 오후 1:25:20
 *  Author: user
 */ 

#include "button.h"


// 버튼 초기화 방향설정(입력) 
void init_button(void)
{
	BUTTON_DDR &= ~(1 << BUTTON0PIN);
	BUTTON_PORT |= (1 << BUTTON0PIN);
}

int get_button(int button_num, int button_pin)
{
	static unsigned char button_status[1] =
	{
		BUTTON_RELEASE
	};
	int current_state;
	
	// 1. button을 읽는다. 
	current_state = BUTTON_PIN & (1 << button_pin);
	// 2. 버튼 상태 check 
	if (current_state  && button_status[button_num] == BUTTON_RELEASE)  
	{
		_delay_ms(10); 
		button_status[button_num] =BUTTON_PRESS;  
		return 0;    
	}
	else if (button_status[button_num] == BUTTON_PRESS && current_state == BUTTON_RELEASE)
	{  // 버튼이 이전에 눌려진 상태였으며 지금은 떼어진 상태 
		_delay_ms(10);  
		button_status[button_num] = BUTTON_RELEASE;   // 다음 버튼을 체크 하기 위하여 초기화

		return 1;   // 완전히 1번 눌렀다 뗀 상태로 인정 한다. 
	}
	
	return 0;   // 버튼이 open 상태 이다. 
}
