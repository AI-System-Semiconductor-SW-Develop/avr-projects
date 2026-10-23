/*
 * buzzer.c
 *
 * Created: 2026-07-02 오후 1:58:40
 *  Author: kccistc
 */ 

#include "keypad.h"

#define DO_01 955
#define RE_01 850
#define MI_01 757
#define FA_01 715
#define SO_01 637
#define LA_01 567
#define SI_01 505
#define DO_02 477
#define RE_02 425
#define MI_02 378
#define FA_02 357
#define SO_02 318
#define LA_02 283
#define SI_02 245
#define DO_03 210

void init_speaker(void)
{
	DDRE |= 0x08;   // PWM CHANNEL  OC3A(PE3) 출력 모드로 설정 한다.
	TCCR3A = (1<<COM3A0); // COM3A0 : 비교일치시 PE3 출력 반전
	TCCR3B = (1<<WGM32) | (1<<CS31);   // WGM32 : CTC 4 CS31: 8분주
	
	TCCR3C = 0;
	OCR3A = 0;   // 비교 일치값을 OCR3A에 넣는다.
	
	return;
}

void keypad_sound (uint8_t key_data)
{
	switch (key_data)
	{

	case '1':
		OCR3A = RE_01;
		break;
	case '2':
		OCR3A = MI_01;
		break;
	case '3':
		OCR3A = FA_01;
		break;
	case '4':
		OCR3A = LA_01;
		break;
	case '5':
		OCR3A = SI_01;
		break;
	case '6':
		OCR3A = DO_02;
		break;
	case '7':
		OCR3A = MI_02;
		break;
	case '8':
		OCR3A = FA_02;
		break;
	case '9':
		OCR3A = SO_02;
		break;
	case '/':
		OCR3A = DO_01;
		break;
	case '*':
		OCR3A = SO_01;
		break;
	case '-':
		OCR3A = RE_02;
		break;
	case '+':
		OCR3A = LA_02;
		break;
	case '=':
		OCR3A = 399;
		_delay_ms(100);
		OCR3A = 299;
	default:
		OCR3A = 0;
		break;
	}
}