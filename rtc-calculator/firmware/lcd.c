/*
 * lcd.c
 *
 * Created: 2026-07-01 오전 9:45:01
 *  Author: kccistc
 */ 

#include "lcd.h"
#include "button.h"
#include "calculator.h"
#include <util/atomic.h>
#include <math.h>

void init_lcd(void);
void lcd_purse_enable(void);
void lcd_display_data(uint8_t data);
void lcd_clear(void);
void lcd_cmd_exe_4bit_once(uint8_t cmd);
void lcd_cmd_exe_4bit(uint8_t cmd);
void lcd_string(char *str);
void lcd_goto_xy(uint8_t row , uint8_t col);

extern uint8_t get_button (int button_num, int button_pin);

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
	
	// HD44780 instruction 초기화 (데이터시트 초기화 순서)
	// 전원 직후 LCD가 8비트 모드인지, 4비트 모드에서 상위 니블만 받은 상태인지 알 수 없다.
	// 0x30(8비트 설정)을 세 번 보내면 어느 상태에서 시작해도 8비트 모드로 맞춰지고,
	// 그 뒤 0x20 한 니블로 4비트 모드에 들어간다.
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

/*
 * ---------- 계산기 키 입력 큐 ----------
 * 키패드 ISR(20ms 주기)은 key_data를 매번 덮어쓴다. 메인이 LCD를 다시 그리거나 부저를 울리는 동안
 * (수십~수백 ms) 눌린 키는 key_data에서 사라지므로, 계산기 모드에서는 ISR이 큐에 넣고 메인이 꺼낸다.
 * head는 ISR만, tail은 메인만 갱신한다(생산자 1·소비자 1, 8비트 인덱스).
 */
#define CALC_KEY_Q_SIZE	8U
#define CALC_KEY_Q_MASK	(CALC_KEY_Q_SIZE - 1U)

static volatile uint8_t calc_key_q[CALC_KEY_Q_SIZE];
static volatile uint8_t calc_key_head = 0;
static volatile uint8_t calc_key_tail = 0;

void calc_key_push(uint8_t key)		/* TIMER0 ISR 전용 */
{
	uint8_t next = (calc_key_head + 1U) & CALC_KEY_Q_MASK;
	if (next == calc_key_tail)			/* 가득 참: 새 키 버림 */
		return;
	calc_key_q[calc_key_head] = key;
	calc_key_head = next;
}

static uint8_t calc_key_pop(void)		/* 메인 전용, 비어 있으면 0 */
{
	uint8_t key;
	if (calc_key_tail == calc_key_head)
		return 0;
	key = calc_key_q[calc_key_tail];
	calc_key_tail = (calc_key_tail + 1U) & CALC_KEY_Q_MASK;
	return key;
}

static void calc_key_flush(void)
{
	calc_key_tail = calc_key_head;
}

/* ---------- 수식 입력 화면 ---------- */

/* 수식을 처음부터 다시 그린다: 1행 0~15번째, 2행 16~31번째 문자, 커서는 다음 입력 위치 */
static void calc_redraw(const char *str, uint8_t len)
{
	lcd_clear();
	for (uint8_t i = 0; i < len; i++)
	{
		if (i == 16)
			lcd_goto_xy(1, 0);
		lcd_display_data(str[i]);
	}
	if (len < 16)
		lcd_goto_xy(0, len);
	else if (len < CALC_MAX_LEN)
		lcd_goto_xy(1, len - 16);
}

/* 한 글자 추가: 길이 제한 검사, 16번째 다음 글자는 2행으로 이동 */
static void calc_append(char *str, uint8_t *len, char c)
{
	if (*len >= CALC_MAX_LEN)			/* 버퍼·화면이 가득 참: 무시 */
		return;
	if (*len == 16)
		lcd_goto_xy(1, 0);
	str[(*len)++] = c;
	str[*len] = '\0';
	lcd_display_data(c);
}

/* 결과 또는 오류 문구를 1행에 표시 */
static void calc_show_result(const char *str)
{
	char text[17];
	double value;
	calc_status_t status = calculation(str, &value);

	if (status != CALC_OK)
	{
		lcd_clear();
		lcd_goto_xy(0, 0);
		lcd_string((char *)calc_error_message(status));
		return;
	}

	/* 소수 둘째 자리까지, 큰 수는 지수 표기로 16자 안에 맞춘다 */
	if (fabs(value) < 1.0e7)
		dtostrf(value, 0, 2, text);		/* 최대 "-9999999.99" (11자) */
	else
		dtostre(value, text, 4, 0);		/* 예: "-1.2346e+12" (11자) */

	lcd_clear();
	lcd_goto_xy(0, 0);
	lcd_string(text);
}

uint8_t lcd_cal (void)
{
	char str[CALC_MAX_LEN + 1] = "";	/* LCD 두 줄(32자) + '\0' */
	uint8_t str_len = 0;
	uint8_t showing_result = 0;			/* '=' 직후: 다음 입력은 새 수식으로 시작 */

	calc_key_flush();					/* 진입 전에 눌린 키 제거 */

	while (1)
	{
		char input = 0;

		if (sec_count >= 60) {			/* 60초 미입력: 시계 모드로 복귀 */
			mode = 0;
			sec_count = 0;
			break;
		}

		// 0번 버튼: 수식 전체 지우기
		if (get_button(BUTTON0,BUTTON0PIN))
		{
			lcd_clear();
			str_len = 0;
			str[0] = '\0';
			showing_result = 0;
			sec_count = 0;
			continue;
		}

		// 1번 버튼: 한 글자 지우기
		if (get_button(BUTTON1,BUTTON1PIN))
		{
			sec_count = 0;
			if (showing_result)			/* 결과 화면에서는 수식을 다시 보여 줌 */
			{
				showing_result = 0;
				calc_redraw(str, str_len);
			}
			else if (str_len > 0)
			{
				str[--str_len] = '\0';
				calc_redraw(str, str_len);
			}
			continue;
		}

		// 2번 / 3번 버튼: 여는 / 닫는 괄호 (키패드에 괄호가 없어 버튼에 배정)
		// get_button()은 눌린 순간 한 번만 1을 반환하므로 뗄 때까지 기다릴 필요가 없다.
		if (get_button(BUTTON2,BUTTON2PIN))
			input = '(';
		else if (get_button(BUTTON3,BUTTON3PIN))
			input = ')';
		else
			input = (char)calc_key_pop();	/* ISR 큐에서 한 번만 꺼내 지역 변수로 사용 */

		if (input == 0)
			continue;

		sec_count = 0;

		if (input == '=')
		{
			calc_show_result(str);
			showing_result = 1;
			continue;
		}

		if (showing_result)				/* 결과가 떠 있는 상태에서 새 입력: 새 수식 시작 */
		{
			showing_result = 0;
			str_len = 0;
			str[0] = '\0';
			lcd_clear();
		}

		calc_append(str, &str_len, input);
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