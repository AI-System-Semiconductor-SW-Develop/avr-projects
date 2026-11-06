/*
 * calculator.c
 *
 * Created: 2026-07-01 오전 9:42:11
 *  Author: kccistc
 *
 * 숫자 스택과 연산자 스택 두 개로 중위식을 바로 계산한다.
 * 수정 사항
 *  - 숫자 누적 변수: int(AVR에서 16비트, 최대 32767) → int32_t, 8자리 이상은 범위 오류
 *  - 잘못된 수식, 괄호 불일치, 0으로 나누기, 스택 초과를 오류 코드로 돌려준다
 *    (이전에는 빈 스택에서 0을 꺼내 엉뚱한 값을 결과로 표시했다)
 *  - 단항 마이너스 지원: -3+5, 2*-3, -(2+3), 1--2
 */

#include "calculator.h"
#include <math.h>

#define CALC_NUM_MAX	9999999L	/* 7자리까지: AVR double(32비트 float)은 약 1677만까지 정수를 정확히 표현 */
#define CALC_RESULT_MAX	1.0e30		/* AVR double(32비트 float) 최대 약 3.4e38 */

/* ---------- 연산자 스택 ---------- */

static void init_opstack(opstack *s)	{ s->top = -1; }
static int  op_empty(const opstack *s)	{ return s->top < 0; }

static int op_push(opstack *s, char item)
{
	if (s->top >= STACK_SIZE - 1)
		return -1;
	s->op_data[++s->top] = item;
	return 0;
}

static char op_pop(opstack *s)
{
	return op_empty(s) ? 0 : s->op_data[s->top--];
}

static char op_peek(const opstack *s)
{
	return op_empty(s) ? 0 : s->op_data[s->top];
}

/* ---------- 숫자 스택 ---------- */

static void init_numstack(numstack *s)	{ s->top = -1; }
static int  num_count(const numstack *s)	{ return s->top + 1; }

static int num_push(numstack *s, double item)
{
	if (s->top >= STACK_SIZE - 1)
		return -1;
	s->num_data[++s->top] = item;
	return 0;
}

static double num_pop(numstack *s)
{
	return (s->top < 0) ? 0 : s->num_data[s->top--];
}

/* ---------- 계산 ---------- */

static int priority(char op)
{
	switch (op)
	{
	case '(':           return 0;	/* 가장 낮게: 괄호 안에서 바깥 연산자를 먼저 꺼내지 않음 */
	case '+': case '-': return 1;
	case '*': case '/': return 2;
	}
	return -1;
}

/* 연산자 하나를 꺼내 숫자 두 개로 계산하고 결과를 다시 넣는다 */
static calc_status_t reduce_once(numstack *nums, opstack *ops)
{
	if (num_count(nums) < 2 || op_empty(ops))
		return CALC_ERR_SYNTAX;

	double b = num_pop(nums);
	double a = num_pop(nums);
	double result;

	switch (op_pop(ops))
	{
	case '+': result = a + b; break;
	case '-': result = a - b; break;
	case '*': result = a * b; break;
	case '/':
		if (b == 0)
			return CALC_ERR_DIV0;
		result = a / b;
		break;
	default:
		return CALC_ERR_SYNTAX;		/* '('가 남아 있는 경우 등 */
	}

	if (isnan(result) || isinf(result) || fabs(result) > CALC_RESULT_MAX)
		return CALC_ERR_RANGE;

	num_push(nums, result);			/* 방금 두 개를 꺼냈으므로 공간이 있음 */
	return CALC_OK;
}

calc_status_t calculation(const char *formula, double *out)
{
	opstack ops;
	numstack nums;
	calc_status_t status;

	int32_t value = 0;
	uint8_t take_num = 0;		/* 숫자를 누적하는 중 */
	uint8_t negative = 0;		/* 단항 마이너스가 붙은 상태 */
	uint8_t expect_operand = 1;	/* 1: 숫자나 '('가 와야 함, 0: 연산자나 ')'가 와야 함 */
	uint8_t paren_depth = 0;

	init_opstack(&ops);
	init_numstack(&nums);

	for (uint8_t i = 0; formula[i] != '\0'; i++)
	{
		char key = formula[i];

		/* 숫자: 자릿수 누적 */
		if (key >= '0' && key <= '9')
		{
			if (!expect_operand && !take_num)		/* ")3" */
				return CALC_ERR_SYNTAX;
			if (value > (CALC_NUM_MAX - (key - '0')) / 10)
				return CALC_ERR_RANGE;
			value = value * 10 + (key - '0');
			take_num = 1;
			continue;
		}

		if (key == ' ')			/* 키패드의 빈 칸 키는 무시 */
			continue;

		/* 숫자가 끝났으면 숫자 스택에 넣는다 */
		if (take_num)
		{
			if (num_push(&nums, negative ? -(double)value : (double)value) < 0)
				return CALC_ERR_RANGE;
			value = 0;
			take_num = 0;
			negative = 0;
			expect_operand = 0;
		}

		if (key == '(')
		{
			if (!expect_operand)					/* "2(3)", ")(" */
				return CALC_ERR_SYNTAX;
			if (negative)							/* -(...) = -1 * (...) */
			{
				if (num_push(&nums, -1) < 0 || op_push(&ops, '*') < 0)
					return CALC_ERR_RANGE;
				negative = 0;
			}
			if (op_push(&ops, '(') < 0)
				return CALC_ERR_RANGE;
			paren_depth++;
		}
		else if (key == ')')
		{
			if (expect_operand)					/* "()", "(2+)" */
				return CALC_ERR_SYNTAX;
			if (paren_depth == 0)					/* 여는 괄호 없음 */
				return CALC_ERR_PAREN;
			/* 여는 괄호가 나올 때까지 반복 계산 */
			while (op_peek(&ops) != '(')
				if ((status = reduce_once(&nums, &ops)) != CALC_OK)
					return status;
			op_pop(&ops);							/* 여는 괄호 버리기 */
			paren_depth--;
		}
		else if (key == '+' || key == '-' || key == '*' || key == '/')
		{
			if (expect_operand)
			{
				if (key == '-' && !negative)		/* 단항 마이너스 */
				{
					negative = 1;
					continue;
				}
				return CALC_ERR_SYNTAX;			/* "*3", "2+*3", "1---2" */
			}
			/* 스택 위 연산자의 우선순위가 같거나 높으면 먼저 계산 (왼쪽 결합) */
			while (!op_empty(&ops) && priority(op_peek(&ops)) >= priority(key))
				if ((status = reduce_once(&nums, &ops)) != CALC_OK)
					return status;
			if (op_push(&ops, key) < 0)
				return CALC_ERR_RANGE;
			expect_operand = 1;
		}
		else
		{
			return CALC_ERR_SYNTAX;				/* 알 수 없는 문자 */
		}
	}

	/* 마지막 숫자 */
	if (take_num)
	{
		if (num_push(&nums, negative ? -(double)value : (double)value) < 0)
			return CALC_ERR_RANGE;
		expect_operand = 0;
	}

	if (expect_operand)							/* 빈 수식, 연산자로 끝남, "-" */
		return CALC_ERR_SYNTAX;
	if (paren_depth != 0)						/* 닫히지 않은 괄호 */
		return CALC_ERR_PAREN;

	/* 남은 연산 모두 계산 */
	while (!op_empty(&ops))
		if ((status = reduce_once(&nums, &ops)) != CALC_OK)
			return status;

	if (num_count(&nums) != 1)
		return CALC_ERR_SYNTAX;

	*out = num_pop(&nums);
	return CALC_OK;
}

/* LCD 1행(16자)에 들어가는 오류 문구 */
const char *calc_error_message(calc_status_t status)
{
	switch (status)
	{
	case CALC_ERR_SYNTAX: return "Error: syntax";
	case CALC_ERR_PAREN:  return "Error: ( )";
	case CALC_ERR_DIV0:   return "Error: div by 0";
	case CALC_ERR_RANGE:  return "Error: range";
	default:              return "";
	}
}
