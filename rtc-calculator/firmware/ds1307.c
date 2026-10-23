/*
 * ds1307.c
 *
 * Created: 2026-07-01 오후 12:56:52
 *  Author: kccistc
 */ 

#include "ds1307.h"
#include "button.h"


uint8_t dec2bcd(uint8_t data);
uint8_t bcd2dec(uint8_t data);
void i2c_clock(int *mode, int *time, char* line);
void init_i2c(void);
void i2c_calender(int *mode, int *time, char *line);

void change_clock_mode(int *mode, int*time, char *change_buff);
void change_clock(uint8_t addr, uint8_t data);
uint8_t get_last_date(uint8_t year, uint8_t month);
uint8_t calculate_day(uint8_t year, uint8_t month, uint8_t date);
volatile uint8_t key_event = 0;

extern uint8_t scan_keypad (void);
extern uint8_t get_button(int button_num, int button_pin);
extern void lcd_display_data(uint8_t data);
extern void lcd_clear(void);
extern void lcd_cmd_exe_4bit(uint8_t cmd);
extern void lcd_string(char *str);
extern volatile uint8_t key_data;
extern volatile int mode;
// dec --> bcd
// 예) 25
// dec 00011001
// bcd 00100101


const char* dayNames[] = {
	"",
	"SUNDAY",
	"MONDAY",
	"TUESDAY",
	"WEDNESDAY",
	"THURSDAY",
	"FRIDAY",
	"SATURDAY"
};

uint8_t dec2bcd(uint8_t data) {
	uint8_t high, low;
	
	high = (data / 10) << 4; // high nibble에 위치
	low = data % 10;
	
	return (high + low);
}

// 1. 입력 bcd
// 예) 26년의 bcd
// 7654 3210
// 0010 0110
// * 10
// 26
uint8_t bcd2dec(uint8_t data) {
	uint8_t high, low;
	
	low = data & 0x0f;
	high = (data >> 4) * 10;
	
	return (high+low);
}

// TWI(I2C) driver
void init_i2c(void)
{
	DDRD &= ~((1 << PD0) | (1 << PD1));
	//PORTD &= ~((1 << PD0) | (1 << PD1));
    PORTD |= (1 << PD0) | (1 << PD1);
	TWSR = 0x00;
	TWBR = TWBR_VALUE;
	TWCR = (1 << TWEN);
}

uint8_t i2c_start(void)
{
    uint8_t st;

    TWCR = (1 << TWINT) |
    (1 << TWSTA) |
    (1 << TWEN);

    // 최대 약 100ms
    for (uint16_t i = 0; i < 10000; i++)
    {
	    if (TWCR & (1 << TWINT))
	    {
		    st = TWSR & 0xF8;

		    if (st == 0x08 || st == 0x10)
		    return 0;

		    return st;
	    }

	    _delay_us(10);
    }

    // 타임아웃 경계에서 완료됐는지 한 번 더 확인
    if (TWCR & (1 << TWINT))
    {
	    st = TWSR & 0xF8;

	    if (st == 0x08 || st == 0x10)
	    return 0;

	    return st;
    }

    return 0xFF;
}

void i2c_stop(void) {
	
	TWCR = 1 << TWINT | 1 << TWSTO | 1 << TWEN;
	// TWINT		TWSTA : STOP 요청	TWEN: I2C HW 활성화

	_delay_us(10);

}

uint8_t i2c_write_byte(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);

	while (!(TWCR & (1 << TWINT)));

	return TWSR & 0xF8;
}


char get_keypad_pressed(void)
{
	static uint8_t key_locked = 0;
	static uint8_t release_count = 0;

	char key = scan_keypad();

	if (key >= '0' && key <= '9') {
		release_count = 0;

		if (!key_locked) {
			key_locked = 1;
			return key;
		}
		} else {
		if (release_count < 2)
		release_count++;

		if (release_count >= 2)
		key_locked = 0;
	}

	return 0;
}

void update_keypad(void)
{
	uint8_t scanned_key;

	scanned_key = scan_keypad();

	/* 계산기에서 기존처럼 사용 */
	key_data = scanned_key;

	/* 시간/날짜 설정 모드에서는 입력을 저장 */
	if (mode == 2 || mode == 3) {
		if (scanned_key != 0 && key_event == 0) {
			key_event = scanned_key;
		}
		} else {
		key_event = 0;
	}
}
void change_clock_mode(int *mode, int *time, char *change_buff)
{
	static uint8_t i = 0;
	static uint16_t blink_cnt = 0;
	static uint8_t blink = 1;

	const uint8_t clk_table[6] = {0, 1, 3, 4, 6, 7};

	char lcd_buff[9];
	uint8_t keypad_char;

	/* 커서 깜빡임 */
	blink_cnt++;

	if (blink_cnt >= 5) {
		blink_cnt = 0;
		blink = !blink;
	}

	strcpy(lcd_buff, change_buff);

	if (i < 6 && blink) {
		lcd_buff[clk_table[i]] = '_';
	}

	lcd_cmd_exe_4bit(0x80);
	lcd_string("SET TIME        ");

	lcd_cmd_exe_4bit(0xC0);
	lcd_string(lcd_buff);

	/* 저장된 키패드 입력 가져오기 */
	keypad_char = key_event;

	if (keypad_char != 0) {
		key_event = 0;

		if (keypad_char >= '0' &&
		keypad_char <= '9' &&
		i < 6) {

			change_buff[clk_table[i]] = keypad_char;
			i++;

			blink_cnt = 0;
			blink = 0;
		}
	}

	/* BUTTON2: 날짜 설정 화면으로 이동 */
	if (get_button(BUTTON2, BUTTON2PIN)) {
		sprintf(
		change_buff,
		"20%02d/%02d/%02d",
		time[6],
		time[5],
		time[4]
		);

		i = 0;
		blink_cnt = 0;
		blink = 1;
		key_event = 0;

		lcd_clear();
		*mode = 3;

		_delay_ms(100);
		return;
	}

	/* BUTTON0: 시간 변경 완료 */
	if (get_button(BUTTON0, BUTTON0PIN)) {
		uint8_t hour;
		uint8_t min;
		uint8_t sec;

		hour =
		(change_buff[0] - '0') * 10 +
		(change_buff[1] - '0');

		min =
		(change_buff[3] - '0') * 10 +
		(change_buff[4] - '0');

		sec =
		(change_buff[6] - '0') * 10 +
		(change_buff[7] - '0');

		if (hour < 24 && min < 60 && sec < 60) {
			change_clock(HOURS, hour);
			change_clock(MINUTES, min);
			change_clock(SECONDS, sec);

			i = 0;
			blink_cnt = 0;
			blink = 1;
			key_event = 0;

			*mode = 0;
			lcd_clear();
			return;
		}

		lcd_cmd_exe_4bit(0xC0);
		lcd_string("Invalid Time    ");

		i = 0;
		blink_cnt = 0;
		blink = 1;
		key_event = 0;

		_delay_ms(300);
		lcd_clear();
	}

	/* BUTTON3: 취소 */
	if (get_button(BUTTON3, BUTTON3PIN)) {
		i = 0;
		blink_cnt = 0;
		blink = 1;
		key_event = 0;

		*mode = 0;
		lcd_clear();
		return;
	}

	_delay_ms(10);
}
void change_calender_mode(int *mode, int *time, char *change_buff)
{
	static uint8_t i = 0;
	static uint16_t blink_cnt = 0;
	static uint8_t blink = 1;

	const uint8_t clk_table[6] = {2, 3, 5, 6, 8, 9};

	char lcd_buff[11];
	uint8_t keypad_char;

	/* 커서 깜빡임 */
	blink_cnt++;

	if (blink_cnt >= 5) {
		blink_cnt = 0;
		blink = !blink;
	}

	strcpy(lcd_buff, change_buff);

	if (i < 6 && blink) {
		lcd_buff[clk_table[i]] = '_';
	}

	lcd_cmd_exe_4bit(0x80);
	lcd_string("SET DATE        ");

	lcd_cmd_exe_4bit(0xC0);
	lcd_string(lcd_buff);

	/* 저장된 키패드 입력 가져오기 */
	keypad_char = key_event;

	if (keypad_char != 0) {
		key_event = 0;

		if (keypad_char >= '0' &&
		keypad_char <= '9' &&
		i < 6) {

			change_buff[clk_table[i]] = keypad_char;
			i++;

			blink_cnt = 0;
			blink = 0;
		}
	}

	/* BUTTON2: 시간 설정 화면으로 이동 */
	if (get_button(BUTTON2, BUTTON2PIN)) {
		sprintf(
		change_buff,
		"%02d:%02d:%02d",
		time[2],
		time[1],
		time[0]
		);

		i = 0;
		blink_cnt = 0;
		blink = 1;
		key_event = 0;

		lcd_clear();
		*mode = 2;

		_delay_ms(100);
		return;
	}

	/* BUTTON0: 날짜 변경 완료 */
	if (get_button(BUTTON0, BUTTON0PIN)) {
		uint8_t year;
		uint8_t month;
		uint8_t date;
		uint8_t day;

		/* change_buff 형식: "20YY/MM/DD" */
		year =
		(change_buff[2] - '0') * 10 +
		(change_buff[3] - '0');

		month =
		(change_buff[5] - '0') * 10 +
		(change_buff[6] - '0');

		date =
		(change_buff[8] - '0') * 10 +
		(change_buff[9] - '0');

		if (month >= 1 &&
		month <= 12 &&
		date >= 1 &&
		date <= get_last_date(year, month)) {

			day = calculate_day(year, month, date);

			change_clock(YEAR, year);
			change_clock(MONTH, month);
			change_clock(DATE, date);
			change_clock(DAYOFWEEK, day);

			i = 0;
			blink_cnt = 0;
			blink = 1;
			key_event = 0;

			*mode = 1;
			lcd_clear();
			return;
		}

		lcd_cmd_exe_4bit(0xC0);
		lcd_string("Invalid Date    ");

		i = 0;
		blink_cnt = 0;
		blink = 1;
		key_event = 0;

		_delay_ms(300);
		lcd_clear();
	}

	/* BUTTON3: 취소 */
	if (get_button(BUTTON3, BUTTON3PIN)) {
		i = 0;
		blink_cnt = 0;
		blink = 1;
		key_event = 0;

		*mode = 0;
		lcd_clear();
		return;
	}

	_delay_ms(10);
}
void change_clock(uint8_t addr, uint8_t data) {
	i2c_start();

	i2c_write_byte(SLAVE_ADDR << 1);   // Write
	i2c_write_byte(addr);               // 주소
	i2c_write_byte(dec2bcd(data));    // 입력할 데이터 값

	i2c_stop();
}

uint8_t i2c_data_read_ack_send(void) {
	TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA;	// TWEA=1 ACK 자동 전송
	
	while ( !(TWCR & (1 << TWINT)));
	return (TWDR);
}

uint8_t i2c_data_read_nack_send(void) {
	TWCR = 1 << TWINT | 1 << TWEN;	// TWEA=1 NACK 자동 전송
	
	while ( !(TWCR & (1 << TWINT)));
	return (TWDR);
}

void init_clock(void) {
	uint8_t data[] = {00, 18, 18, 4, 2, 7, 26}; // 바꿀 날자
		
	i2c_start();
	i2c_write_byte(SLAVE_ADDR << 1); // I2C 주소 전송. 쓰기모드
	i2c_write_byte(0); // 주소전달
	
	printf("Setting RTC module...\r \n");
	for(int i=0;i<7;i++) {
		printf("%dth byte written...\r\n", i);
		
		i2c_write_byte(dec2bcd(data[i])); // 시간설정
	}
	
	i2c_stop();
	
	_delay_ms(2000); // 2초대기
	
	i2c_start();
	i2c_write_byte(SLAVE_ADDR << 1); // I2C 주소 전송. 쓰기모드
	i2c_write_byte(0); // 주소전달
	i2c_stop();
	
	i2c_start();
	i2c_write_byte((SLAVE_ADDR << 1) + 1); // I2C 주소 전송. 읽기모드
	
	printf("Time / DATE Retrieval... \r \n");

	i2c_stop();
	
}

void save_time(int *temp) {
		temp[0] = bcd2dec(i2c_data_read_ack_send() & 0x7F); // second  CH를 없애기 위한 & 0x7F
		temp[1] = bcd2dec(i2c_data_read_ack_send());        // minute
		temp[2] = bcd2dec(i2c_data_read_ack_send() & 0x3F); // hour		앞비트를 쓰지 않는다.
		temp[3] = bcd2dec(i2c_data_read_ack_send());        // day
		temp[4] = bcd2dec(i2c_data_read_ack_send());        // date
		temp[5] = bcd2dec(i2c_data_read_ack_send());        // month
		temp[6] = bcd2dec(i2c_data_read_nack_send());       // year
}

void i2c_clock(int *mode, int *time, char* line)
{
	static int prev_sec = 60; // 초에 60은 절대 없으니 이렇게 초기화

	i2c_start();
	i2c_write_byte(SLAVE_ADDR << 1);
	i2c_write_byte(0x00);

	i2c_start();
	i2c_write_byte((SLAVE_ADDR << 1) | 1);

	save_time(time);
	i2c_stop();

	if (time[0] != prev_sec) { // 현재 초가 전 초와 다르면 변경
		prev_sec = time[0];
		sprintf(line, "%02d / %02d %s", time[5], time[4], dayNames[time[3]]);
		lcd_cmd_exe_4bit(0x80);   // LCD 첫 번째 줄 첫 칸으로 이동
		lcd_string(line);
		
		sprintf(line, "%02d : %02d : %02d", time[2], time[1], time[0]);
		lcd_cmd_exe_4bit(0xc0);
		lcd_string(line);
	}

	_delay_ms(100);
}

void i2c_calender(int *mode, int *time, char *line) {
	static int prev_date = 32;
	
	i2c_start();
	i2c_write_byte(SLAVE_ADDR << 1);
	i2c_write_byte(0x00);

	i2c_start();
	i2c_write_byte((SLAVE_ADDR << 1) | 1);
	
	save_time(time);
	i2c_stop();

	if (time[0] != prev_date) { // 현재 초가 전 초와 다르면 변경
		prev_date = time[4];
		sprintf(line, "20%02d / %02d / %02d", time[6], time[5], time[4]);

		lcd_cmd_exe_4bit(0x80);   // LCD 첫 번째 줄 첫 칸으로 이동
		lcd_string(line);
		
		sprintf(line, "%s", dayNames[time[3]]);
		lcd_cmd_exe_4bit(0xc0);	  // LCD 두 번째 줄 첫 칸으로 이동
		lcd_string(line);
	}

	_delay_ms(100);
}

// 해당 월의 마지막 날짜 계산
uint8_t get_last_date(uint8_t year, uint8_t month) {
	static const uint8_t last_date[12] = {
		31, 28, 31, 30, 31, 30,
		31, 31, 30, 31, 30, 31
	};

	uint16_t full_year = 2000 + year;
	uint8_t result = last_date[month - 1];

	// 윤년 처리
	if (month == 2) {
		if ((full_year % 400 == 0) ||
		(full_year % 4 == 0 &&
		full_year % 100 != 0)) {
			result = 29;
		}
	}

	return result;
}


// 1=일요일, 2=월요일, ..., 7=토요일
uint8_t calculate_day(uint8_t year, uint8_t month, uint8_t date){
	static const uint8_t month_table[12] = {
		0, 3, 2, 5, 0, 3,
		5, 1, 4, 6, 2, 4
	};

	uint16_t full_year = 2000 + year;

	if (month < 3) {
		full_year--;
	}

	return (
	full_year
	+ full_year / 4
	- full_year / 100
	+ full_year / 400
	+ month_table[month - 1]
	+ date
	) % 7 + 1;
}