/*
 * i2c_m_loopback.c
 *
 * Created: 2026-06-30 오전 10:53:01
 *  Author: kccistc
	"ABC" 문자열을 slave로 전송하고 slave가 돌려보낸 문자열을 수신하여 UART로 출력 하는 loop-back test program을 작성한다
	ATmega128A 2개를 연결한다. 1개는 master 1개는 slaver로 동작하도록 한다.
 
	H/W 조건
	========
	1. I2C 연결
	PD0 (SCL) -- 10K(pullup 저항) --- VCC
	PD1 (SDA) -- 10K(pullup 저항) --- VCC
	두 atmega의 GND 라인은 공통으로 연결한다. VCC 끼리는 공통으로 연결 안해도 된다.
	
	I2C Addr : 0x51
	
	2. 속도
	F_CPU: 16MHz I2C속도: 100KHz UART: 9600Bps
	
	3. Handshaking
	ACK / NACK 의 의미
	
	신호		SDA(데이터 라인)		Slave 입장에서의 의미
	================================================
	ACK		LOW					다음 바이트도 계속 보낸다. 정상적으로 수신 의미도 있다.
	NACK	HIGH				이제 그만 보내라 전송 끝이다.
 */ 

#include "i2c_m_loopback.h"

int i2c_main(void);

// TWI(I2C) driver
void init_i2c(void)
{
	DDRD &= ~((1 << PD0) | (1 << PD1));
	PORTD &= ~((1 << PD0) | (1 << PD1));

	TWSR = 0x00;
	TWBR = 64;
	TWCR = (1 << TWEN);
}

uint8_t i2c_start(void) {
	uint8_t st;
	
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;
	// TWINT		TWSTA : START 요청	TWEN: I2C HW 활성화
	// I2C HW가 동작이 완료 될 때까지 기다린다. 동작이 완료되면 TWINT 프래그를 1로 세트가 된다.
	while( !(TWCR & (1 << TWINT)));
	st = TWSR & 0xf8;	// 7~3 bit까지 값을 취함
	
	return ((st == TWSR_START) ? 0 : st); // 성공이면 0 아니면 error 상태 코드 반환

}

void i2c_stop(void) {
	
	TWCR = 1 << TWINT | 1 << TWSTO | 1 << TWEN;
	// TWINT		TWSTA : STOP 요청	TWEN: I2C HW 활성화

	_delay_us(10);

}
 
void i2c_data_write(uint8_t data) {
	TWDR = data;
	TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA; 
	
	while ( !(TWCR & (1 << TWINT)));
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

void loopback_test(void) {
	char tx_msg[] = "ABC";
	char rx_buff[4] = {0};
	uint8_t status;
	
	//------------------------
	// PHASE#1 Master --> slave전송
	//------------------------
	// 1. start 전송
	status = i2c_start();
	if (status) {
		printf("ERR : START FAILED !!!!\n");
		return ;
	}
	
	// 2. SLAVE_ADDR+W 전송
	status = i2c_slave_addr_send(SLAVE_ADDR << 1); // 0xC0 : SLAVE_ADDR+W
	if (status != TWSR_MT_SLA_ACK) {
		printf("ERR : SLAVE_ADDR + NACK FAILED !!!!\n");
		i2c_stop();
		return ;
	}
	
	// 3. DATA 3byte write
	for (int i=0; tx_msg[i] != '\0';i++) {
		status = i2c_data_write(tx_msg[i]);
		if (status != TWSR_MT_DATA_ACK){
			printf("ERR : DATA_WRITE + NACK!!!!\n");
			i2c_stop();
			break;
		}
	}
	
	// 4. STOP signal
	i2c_stop();
	// START 0xc0 ACK 'A' ACK 'B' ACK 'C' ACK STOP
	
	// 5. 대기 _delay_ms(5);
	_delay_ms(5); // slave 처리 시간
	//---------------------------------
	// PHASE#2: Master <--- slave
	//---------------------------------
	// 1. start
	status = i2c_start();
	if (status) {
		printf("ERR : START(read) failed !!!! \n");
		return ;
	}
	// 2. SLAVE_ADDR+R 전송
	status = i2c_slave_addr_send((SLAVE_ADDR << 1) | 1); // 0xC1 : SLAVE_ADDR+R
	if (status != TWSR_MR_SLA_ACK) {
		printf("ERR : SLAVE_ADDR(read) + NACK FAILED !!!!\n");
		i2c_stop();
		return ;
	}
	// 3. 3byte 수신 A, B, C
	rx_buff[0] = i2c_data_read_ack_send();	// 'A' 수신 + ACK 전송('B'도보내)	
	rx_buff[1] = i2c_data_read_ack_send();	// 'B' 수신 + ACK 전송('C'도보내)
	rx_buff[2] = i2c_data_read_nack_send();	// 'C' 수신 + NACK 전송(그만보내)
	// 4. i2c stop
	i2c_stop();
	
	//// 결과 출력 ////
	printf("TX : %s, RX : %s \n", tx_msg, rx_buff);
	if (strncmp(tx_msg, rx_buff, 3) == 0)
		printf("LOOP BACK OK !!!\n");
	else printf("LOOP BACK FAILED !!!\n");
		
}

//----------------------------
// loop back test : "ABC" 전송
//----------------------------
int i2c_main(void) {
	init_i2c();
	
	printf("I2C loopback test start !!!\n");
	while(1) {
		loopback_test();
		_delay_ms(1000);
	}
}