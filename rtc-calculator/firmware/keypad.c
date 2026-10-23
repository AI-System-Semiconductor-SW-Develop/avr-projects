/*
 * keypad.c
 *
 * Created: 2026-07-01 오전 9:48:41
 *  Author: kccistc
 */ 

#include "keypad.h"

void init_keypad (void)
{
	KEYPAD_DDR = 0x0f; // col : 출력 row: 입력 ( col이 보낸 신호를 읽음 )
	KEYPAD_PORT = 0xff; 
}

uint8_t get_keypad ( int row, int col)
{
	uint8_t keypad_char[4][4] = 
	{
		{'/','3','2','1'},
		{'*','6','5','4'},
		{'-','9','8','7'},
		{'+','=','0',' '}
	};
	
	static uint8_t prev_st[4][4] =
	{
		{1,1,1,1},
		{1,1,1,1},
		{1,1,1,1},
		{1,1,1,1}
	};
	
	int curr_st = 1;
	
	KEYPAD_PORT = 0xff;
	
	KEYPAD_PORT &= ~(1 << 3 - col);
	
	for (int delay = 0; delay < 20; delay++) //keypad check를 위한 delay
	{
		;
	}
	
	curr_st = (KEYPAD_PIN & 1 << (row+4)) >> (row+4);
	
	if(curr_st == 0 && prev_st[row][col]==1)
	{
		prev_st[row][col]=0;
		return 0;
	}
	else if(curr_st == 1 && prev_st[row][col]==0)
	{
		prev_st[row][col]=1;
		return keypad_char[row][col];
	}
	
	return 0;	
}

uint8_t scan_keypad (void)
{	
	uint8_t data = 0;
	
	for (int row = 0; row < 4; row++ )
	{
		for (int col = 0; col < 4; col++)
		{
			data = get_keypad(row,col);
			
			if(data != 0)
				return data;
		}
	}
	return 0;
}

