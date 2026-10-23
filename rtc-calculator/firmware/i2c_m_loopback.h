/*
 * I2c_master_loopback.h
 *
 * Created: 2026-06-30 오전 10:52:35
 *  Author: kccistc
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

//I2C 설정
#define SLAVE_ADDR 0x68 // 7bit salve 주소
#define I2C_FREQUENCY 200000UL // 200KHz
// TWBR : Two Wire(I2C) Bit Rate 설정
// TWBR : (F_CPU / I2C_FREQUENCY - 16) / 2 (분주비 = 1)
#define TWBR_VALUE ((F_CPU / I2C_FREQUENCY - 16) / 2) // TWBR(TWI Bit Rate Register)

//------------ TWI 상태 코드 ---------------
//TWSR(TWI Status Register) : 7번~3번 bit(5bit를 참조) : I2C통신의 전송 상태 오류를 나타낸다.
#define TWSR_START 0x08 // START 조건 전송 완료
#define TWSR_MT_SLA_ACK 0x18 // MASTER가 SLAVE ADDR + W 전송 후 ACK 수신
#define TWSR_MT_DATA_ACK 0x28 // MASTER가 SLAVE DATA 전송 후 ACK 수신
#define TWSR_MR_SLA_ACK 0x40 // MASTER가 SLAVE DATA 전송 후 ACK 수신
#define TWSR_MR_DATA_ACK 0x50 // MASTER가 DATA 수신 뒤 + ACK 전송
#define TWSR_MR_DATA_NACK 0x58 // MASTER가 DATA 수신 뒤 + NACK 전송 (마지막)
/*
	ATmega128A TWI(I2C) 관련 register
	TWCR : TWI Control Register
		-각 BIT의 역할--
		TWINT : 인터럽트 플래그 (1 이면 clear(일반 flag와 다르다)) I2C작업이 끝나면 set
		TWEA : ACK enable (1: 자동으로 ACK가 전송 0: NACK 전송)
		TWSTA : START 신호 발생 (MASTER 전용, SLAVE는 사용 X)
		TWSTO : STOP 신호 발생 (MASTER 전용, SLAVE는 사용 X)
		TWEN : TWI(I2C) enable (I2C 하드웨어를 활성화)
		TWIE : Interrupt enable (TWI(I2C) interrupt를 활성화)
	TWSR : TWI Status Register (상위 5bit == 상태 코드, 하위 2bit : Prescaler
	TWBR : TWI Bit Rate Register
	TWDR : TWI Data Register
	TWAR : TWI Address Register (Slave 모드에서 사용 : 나의 어드레스를 설정)
*/