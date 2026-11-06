/*
 * uart1.c
 *
 * Created: 2026-06-22 오전 10:38:34
 *  Author: user
 */ 

#include "uart1.h"

#include "uart0.h"

/*
 * 블루투스 수신 링 버퍼 (USART1 RX ISR → 메인 루프)
 *  - 변수 1바이트만 쓰면 메인이 처리하기 전에 다음 명령이 앞 명령을 덮고,
 *    "읽기 → 0으로 초기화" 사이에 들어온 명령도 지워진다. S/E처럼 놓치면 안 되는 명령이 있어 큐로 받는다.
 *  - head는 ISR만, tail은 메인만 갱신한다(생산자 1·소비자 1). 인덱스가 8비트라 읽기·쓰기가 원자적이다.
 *  - ISR에서는 UART0 에코(송신 완료 대기)를 하지 않는다. 확인용 에코는 메인 루프에서 한다.
 */
#define BT_RX_SIZE  16U                 // 2의 거듭제곱
#define BT_RX_MASK  (BT_RX_SIZE - 1U)

static volatile uint8_t bt_rx_buf[BT_RX_SIZE];
static volatile uint8_t bt_rx_head = 0; // ISR이 씀
static volatile uint8_t bt_rx_tail = 0; // 메인이 씀
volatile uint8_t bt_rx_overflow = 0;

// P278 표12-3
// BT로부터 1byte가 들어 오면 자동적으로 이곳으로 진입 한다.
ISR(USART1_RX_vect)
{
	uint8_t data = UDR1;                // UDR1을 읽으면 수신 버퍼가 비워진다
	uint8_t next = (bt_rx_head + 1U) & BT_RX_MASK;

	if (next == bt_rx_tail)             // 가득 참: 새 바이트 버림, 사실은 기록
	{
		bt_rx_overflow = 1;
		return;
	}
	bt_rx_buf[bt_rx_head] = data;
	bt_rx_head = next;
}

/* 쌓인 수신 데이터를 모두 버린다 (모드 전환 시, 메인 루프 전용) */
void bt_flush(void)
{
	bt_rx_tail = bt_rx_head;
}

/* 수신된 바이트를 하나 꺼낸다. 비어 있으면 0을 반환 (메인 루프 전용) */
uint8_t bt_read(void)
{
	uint8_t data;

	if (bt_rx_tail == bt_rx_head)
		return 0;

	data = bt_rx_buf[bt_rx_tail];
	bt_rx_tail = (bt_rx_tail + 1U) & BT_RX_MASK;
	return data;
}
/*
1. 전송속도 : 9600bps
2. start/stop 설정
3. RX(수신) : interrupt 로 설정
*/
void init_uart1(void)
{
	// 1. 전송속도 : 9600bps
	UBRR1H = 0x00;
	UBRR1L = 207;   //9600BPS 표8-9
	UCSR1A |= 1 << U2X1;  // 2배속 설정 (sampling 8)
	// UART0 를 송신.수신이 다 가능하고 RX INT가 가능 하도록 설정 한다.
	UCSR1B |= 1 << RXEN1 | 1 << TXEN1 | 1 << RXCIE1;
}

// UART1 로 1byte를 전송 하는 함수
void UART1_transmit(uint8_t data)
{
	while ( !(UCSR1A & 1 << UDRE1))   // data가 송신중이면 송신이 끝날때 까지 기다림
		;     // No operation
	UDR1 = data;   // HW 전송 register에 data를 송신 한다.
}

