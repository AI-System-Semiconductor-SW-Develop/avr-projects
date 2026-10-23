/*
 * lcd.c
 *
 * Created: 2026-07-01 오전 9:45:01
 *  Author: kccistc
 */ 

#include "lcd.h"
#include "button.h"

void init_lcd(void);
void lcd_purse_enable(void);
void lcd_display_data(uint8_t data);
void lcd_clear(void);
void lcd_cmd_exe_4bit_once(uint8_t cmd);
void lcd_cmd_exe_4bit(uint8_t cmd);
void lcd_string(char *str);
void lcd_goto_xy(uint8_t row , uint8_t col);

extern uint8_t get_button (int button_num, int button_pin);
extern double calculation (char *folmualr);

extern volatile uint32_t keypad_counter;
extern volatile uint8_t key_data;
extern volatile int sec_count;
extern int mode;

void lcd_string(char *str)
{
	while (*str)
	{
		lcd_display_data(*str++);
	}
}

void init_lcd(void) {

	LCD_DATA_DDR = 0xf0; // 4bit
	LCD_DATA_PORT = 0x00;
	
	LCD_CONTROL_DDR &= ~(1 << LCD_RS_PIN | 1 << LCD_RW_PIN | 1 << LCD_E_PIN);
	LCD_CONTROL_DDR |= 1 << LCD_RS_PIN | 1 << LCD_RW_PIN | 1 << LCD_E_PIN;
	LCD_CONTROL_PORT &= ~(1 << LCD_RW_PIN); // R/W핀으로 LOW를 출력해 쓰기 전용으로 사용
	
	_delay_ms(15); // 초기 구동 시간
	
	// 0x03을 3번 보내는 이유 현재 하드웨어가 8bit인지 4bit인지, 명령을 어디까지 읽었는지 모르기떄문에
	// 무조건 8bit상태로 리셋시키기 위해 0x30을 3번 보낸다.(왜 3번이나 해야되는지는 모름 걍 doc이 시켰음)
	lcd_cmd_exe_4bit_once(0x30); // 8bit로 초기화
	_delay_us(4100);
	lcd_cmd_exe_4bit_once(0x30);
	_delay_us(100);
	lcd_cmd_exe_4bit_once(0x30);
	_delay_us(100);
	lcd_cmd_exe_4bit_once(0x20);
	lcd_cmd_exe_4bit(0x28);
	
	lcd_cmd_exe_4bit(0x08);
	_delay_us(100);
	lcd_clear(); // 화면 지움
	lcd_cmd_exe_4bit(0x06);
	lcd_cmd_exe_4bit(0x0c);
}

void lcd_purse_enable(void) { // E_PIN이 하강엣지로 내려올 때 동작
	LCD_CONTROL_PORT |= 1 << LCD_E_PIN; // 상승엣지
	_delay_ms(1);
	LCD_CONTROL_PORT &= ~(1 << LCD_E_PIN); // 하강엣지
	_delay_ms(1);
}

void lcd_display_data(uint8_t data) { // 문자출력
	LCD_CONTROL_PORT |= 1 << LCD_RS_PIN; // 문자출력에서는 RS_PIN이 1이어야함
	LCD_DATA_PORT = data & 0xf0; // 상위 4비트
	lcd_purse_enable();
	LCD_DATA_PORT = (data << 4) & 0xf0; // 하위 4비트
	lcd_purse_enable();
	_delay_ms(2);
}

void lcd_cmd_exe_4bit_once(uint8_t cmd) {
	LCD_CONTROL_PORT &= ~(1 << LCD_RS_PIN); // 명령어 입력에서는 RS_PIN이 0이어야함
	LCD_DATA_PORT = cmd & 0xf0;	// 상위 4비트
	lcd_purse_enable();
	_delay_ms(2);
}

void lcd_cmd_exe_4bit(uint8_t cmd) { // 문자출력 이외의 명령어 실행
	LCD_CONTROL_PORT &= ~(1 << LCD_RS_PIN); // 명령어 입력에서는 RS_PIN이 0이어야함
	LCD_DATA_PORT = cmd & 0xf0;	// 상위 4비트
	lcd_purse_enable();
	LCD_DATA_PORT = (cmd << 4) & 0xf0; // 하위 4비트
	lcd_purse_enable();
	_delay_ms(2);
}

void lcd_clear(void) {
	uint8_t display_clear = 0x01;
	
	lcd_cmd_exe_4bit(display_clear);
	_delay_ms(2);
}

uint8_t lcd_cal (void)
{
	char str[50];
	int str_len = 0;
	uint8_t last_key = 0;
	char result[32];
	
	while (1)
	{	
		if (sec_count >= 60) {
			mode = 0;
			sec_count = 0;
			break;
		}
		// 0번 버튼 누르면 초기화
		if (get_button(BUTTON0,BUTTON0PIN))
		{
			lcd_clear();
			key_data =0;
			str_len=0;
			str[0]='\0';
			sec_count = 0;
			continue;
		}
		// 1번 버튼 누르면 한글자 지우기
		if (get_button(BUTTON1,BUTTON1PIN))
		{
			sec_count = 0;
			if (str_len > 0 && str_len <16)
			{
				str_len--;
				str[str_len] = '\0';
				lcd_clear();
				lcd_string(str);
				lcd_goto_xy(0,str_len);
				
				continue;
			}
			else if (str_len >=16)
			{
				str_len--;
				str[str_len] = '\0';
				lcd_clear();
				lcd_string(str);
				lcd_goto_xy(1,str_len);
				continue;
			}
			
		}
		
		// 2번 버튼 누르면 여는 괄호
		if(get_button(BUTTON2,BUTTON2PIN))
		{
			str[str_len++] = '(';
			lcd_display_data('(');
			while(get_button(BUTTON2,BUTTON2PIN));
			_delay_ms(30);
			sec_count = 0;
			continue;
		}
		
		// 1번 버튼 누르면 닫는 괄호
		else if(get_button(BUTTON3,BUTTON3PIN))
		{
			str[str_len++] = ')';
			lcd_display_data(')');
			while(get_button(BUTTON3,BUTTON3PIN));
			_delay_ms(30);
			sec_count = 0;
			continue;
		}
		
		else if ( key_data != 0 && last_key != key_data)
		{
			last_key = key_data;
			
			if (key_data == '=')
			{
				str[str_len] = '\0'; // 배열 끝
				
				dtostrf(calculation(str), 0, 2, result); // 결과 배열에 넣기
				
				lcd_clear();
				lcd_goto_xy(0,0);
				lcd_string(result); // 배열 출력
				
				str_len = 0;
			}
			else
			{
				if (str_len < 16)
				{
					str[str_len++] = key_data;
					lcd_display_data(key_data);
				}
				else if(str_len == 16)
				{
					lcd_goto_xy(1,0);
					str[str_len++] = key_data;
					lcd_display_data(key_data);
					
				}
				else if (str_len > 16)
				{
					str[str_len++] = key_data;
					lcd_display_data(key_data);
					
				}
			}
			sec_count = 0;
		}
		if (key_data == 0)
		{
			last_key = 0;
		}
	}
	return 0;
}

void lcd_goto_xy(uint8_t row , uint8_t col)
{
	col %= 16;
	row %= 2;
	
	//첫째 라인 시작 주소 는 0x00 둘째 라인은 0x40
	uint8_t address = ( 0x40 * row) + col;
	uint8_t command = 0x80 + address;
	
	lcd_cmd_exe_4bit(command);
}