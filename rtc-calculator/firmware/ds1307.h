/*
 * ds1307.h
 *
 * Created: 2026-07-01 오후 12:56:36
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL  //16MHz
#include <avr/io.h>  // PORTA PORTB PORTD... IO관련 reg가 들어있다.
#include <util/delay.h>  // _delay_ms _delay_us 등
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

//I2C 설정
#define SLAVE_ADDR 0x68 // 7bit salve 주소
#define I2C_FREQUENCY 100000UL // 100KHz
#define TWBR_VALUE ((F_CPU / I2C_FREQUENCY - 16) / 2) // TWBR(TWI Bit Rate Register)
#define TWSR_START 0x08 // START 조건 전송 완료
#define TWSR_MT_SLA_ACK 0x18 // MASTER가 SLAVE ADDR + W 전송 후 ACK 수신
#define TWSR_MT_DATA_ACK 0x28 // MASTER가 SLAVE DATA 전송 후 ACK 수신
#define TWSR_MR_SLA_ACK 0x40 // MASTER가 SLAVE DATA 전송 후 ACK 수신
#define TWSR_MR_DATA_ACK 0x50 // MASTER가 DATA 수신 뒤 + ACK 전송
#define TWSR_MR_DATA_NACK 0x58 // MASTER가 DATA 수신 뒤 + NACK 전송 (마지막)

//주소 설정
#define SECONDS 0x00
#define MINUTES 0x01
#define HOURS 0x02
#define DAYOFWEEK 0x03
#define DATE 0x04
#define MONTH 0x05
#define YEAR 0x06