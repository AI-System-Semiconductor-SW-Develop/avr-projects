/*
 * uart0.c
 *
 * Created: 2026-06-16 오전 9:57:59
 *  Author: user
 */ 
#include "uart0.h"

volatile int rear = 0;
volatile int front = 0;
volatile unsigned char rx_buff[QUEUE_SIZE][QUEUE_LENGTH];

//p.278 표 12-3
//pc로부터 1byte가 들어오면 자동적으로 이곳으로 진입 한다.
// 예) led_all_on\n 이면 11번 이곳으로 진입한다.
ISR(USART0_RX_vect)
{
	volatile uint8_t data;
	static volatile int i = 0;
	
	data = UDR0; //UDR0의 내용이 data에 복사 된 후 UDR0의 내용은 빈 상태로 된다.

	if(data == '\n' || data == '\r')
	{
		if((rear+1) % QUEUE_SIZE == front % QUEUE_SIZE) // queue full 상태 체크
		{
			return;
		}
		rx_buff[rear][i] = '\0'; //문장의 끝에 NULL을 넣는다. \n 바꿔치기
		i=0; //다음 string  저장 하기 위해서 i = 0으로 만든다.
		rear = (rear + 1) % QUEUE_SIZE; //0~9
	}
	else
	{
		if((rear+1) % QUEUE_SIZE == front % QUEUE_SIZE) // queue full 상태 체크
		{
			return;
		}
		rx_buff[rear][i++] = data;
	}
}
/*
1. 전송속도 : 9600bps 
2. start/stop 설정
3. RX(수신) : interrupt 로 설정 
*/
void init_uart0(void)
{
	// 1. 전송속도 : 9600bps 
	UBRR0H = 0x00;
	UBRR0L = 207;   //9600BPS 표8-9
	UCSR0A |= 1 << U2X0;  // 2배속 설정 (sampling 8)
	// UART0 를 송신.수신이 다 가능하고 RX INT가 가능 하도록 설정 한다.
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;   
}

// UART0 로 1byte를 전송 하는 함수 
void UART0_transmit(uint8_t data)
{
	while ( !(UCSR0A & 1 << UDRE0))   // data가 송신중이면 송신이 끝날때 까지 기다림
		;     // No operation
	UDR0 = data;   // HW 전송 register에 data를 송신 한다. 
}

/* 기존 바이트 송신 함수를 표준 출력 콜백에 연결한다. */
int uart0_putchar(char data, FILE *stream)
{
    (void)stream;
    UART0_transmit((uint8_t)data);
    return 0;
}
