/*
 * calculator.h
 *
 * Created: 2026-07-01 오전 9:42:34
 *  Author: kccistc
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include "keypad.h"

#define CALC_MAX_LEN	32		/* LCD1602 두 줄(16자 × 2)까지만 입력 */
#define STACK_SIZE		(CALC_MAX_LEN + 1)	/* 수식 길이보다 깊어질 수 없음 */

/* calculation() 반환값 */
typedef enum
{
	CALC_OK = 0,
	CALC_ERR_SYNTAX,	/* 연산자 연속, 빈 수식, 연산자로 끝남, 2(3) 같은 형식 */
	CALC_ERR_PAREN,		/* 괄호 짝이 맞지 않음 */
	CALC_ERR_DIV0,		/* 0으로 나누기 */
	CALC_ERR_RANGE		/* 숫자·결과가 표현 범위를 넘음, 스택 초과 */
} calc_status_t;

typedef struct
{
	char op_data[STACK_SIZE];
	int8_t top;
} opstack;

typedef struct
{
	double num_data[STACK_SIZE];
	int8_t top;
} numstack;

calc_status_t calculation(const char *formula, double *result);
const char *calc_error_message(calc_status_t status);

#endif /* CALCULATOR_H */
