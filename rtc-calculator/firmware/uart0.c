/*
 * uart0.c
 *
 * Created: 2026-06-16 오전 9:57:49
 *  Author: kccistc
 */ 

#include "uart0.h"

//#define FUNC_SU 6
//extern int func_state;
//extern void (*fp[]) ();

void init_uart0(void);
void UART0_transmit(uint8_t data);
//void pc_command_processing(char **led_func_name);

// P278 표12-3
// PC로부터 1byte가 들어 오면 자동적으로 이곳으로 진입한다.
// 예) led_all_on\n 이면 11번 이곳으로 진입한다.
ISR(USART0_RX_vect) {
	volatile uint8_t data;
	volatile static int i = 0;
	
	data = UDR0; // UDR0의 내용이 data에 복사된다.
	
	if ((u_rear+1) % QUEUE_SIZE == u_front % QUEUE_SIZE) // queue full
		return ;
		
	if (data == '\n' || data == '\r') {
		rx_buff[u_rear][i] = '\0'; // 문장의 끝인 NULL을 넣는다.
		i=0; // 다음 string을 저장하기 위해서 i를 0으로 만든다.
		u_rear = (u_rear + 1) % QUEUE_SIZE; // 0~9
	}else {
		
		rx_buff[u_rear][i++] = data;
	}
}
/*
	1. 전송속도 : 9600bps
	2. start/stop 설정
	3. RX(수신) : interrupt로 설정
	TX(전송) : polling으로 처리한다.
	RX(수신) : interrupt로 처리한다.
*/
void init_uart0(void) {
	// 1. 전송속도 : 9600bps
	UBRR0H = 0x00;
	UBRR0L = 207; // 9600bps 표 8-9
	UCSR0A |= 1 << U2X0; // 2배속 설정 (sampling8)
	// UART0를 송신, 수신이 다 가능하고 RX INT가 가능하도록 설정한다.
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
	
}

// UART0로 1byte를 전송 하는 함수
void UART0_transmit(uint8_t data) {
	while (!(UCSR0A & 1 << UDRE0)) // data가 송신 중이면 송신이 끝날 때 까지 기다림
		;	// No operation
	UDR0 = data; // HW 전송 register에 data를 송신한다.
}

void pc_command_processing(char **led_func_name) {
	if (u_front != u_rear) { // data
		printf("%s", rx_buff[u_front]);
		//FOR (INT I=0;I<FUNC_SU;I++) {
			//IF(STRNCMP(RX_BUFF[U_FRONT], LED_FUNC_NAME[I], STRLEN(LED_FUNC_NAME[I])) == 0) {
				////FUNC_STATE = I;
				//BREAK;
			//}
		//}
	}
	
	//fp[func_state]();
}