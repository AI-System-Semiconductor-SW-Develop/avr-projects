/*
 * calculator.c
 *
 * Created: 2026-07-01 오전 9:42:11
 *  Author: kccistc
 */ 

#include "calculator.h"

void init_opstack (opstack *check)
{
	check -> top=-1;
}

int op_empty (opstack *check)
{
	return (check->top == -1);
}

int op_full (opstack *check)
{
	return (check->top == (STACK_SIZE -1));
	
}

void op_push (opstack *check , char item)
{
	if (op_full(check))
	{
		return;
	}
	else 
	{
		check -> top++;
		check ->op_data[check->top] = item;
	}
}

char op_pop (opstack *check)
{
	if (op_empty(check))
	{
		return 0;
	}
	else
	{
		char item = check ->op_data[check->top];
		check -> top--;
		
		return item;
	}
}

char op_peep (opstack *check)
{
	if(op_empty(check))
	{
		return 0;
	}
	else
	{
		char item = check ->op_data[check->top];
		return item;
	}
}

void init_numstack (numstack *check)
{
	check -> top=-1;
}

int num_empty (numstack *check)
{
	return check->top == -1;
}

int num_full (numstack *check)
{
	return check->top == (STACK_SIZE -1);
	
}

void num_push (numstack *check , double item)
{
	if (num_full(check))
	{
		return;
	}
	else
	{
		check -> top++;
		check ->num_data[check->top] = item;
	}
}

double num_pop (numstack *check)
{
	if (num_empty(check))
	{
		return 0;
	}
	else
	{
		double item = check ->num_data[check->top];
		check -> top--;
		
		return item;
	}
}

double num_peep (numstack *check)
{
	if(num_empty(check))
	{
		return 0;
	}
	else
	{
		double item = check ->num_data[check->top];
		return item;
	}
}

int priority (char op)
{
	switch(op)
	{
		case '(': 
			return 0;
		case '+': 
			return 1;
		case '-': 
			return 1;
		case '*': 
			return 2;
		case '/': 
			return 2;
	}
	return -1;
}

double one_cal ( numstack *num_stack , opstack *op_stack)
{
	double b= num_pop(num_stack);
	double a= num_pop(num_stack);
	double result=0;
	
	switch(op_pop(op_stack))
	{
		case '+':
			result = a+b;
			break;
		case '-':
			result = a-b;
			break;
		case '*':
			result = a*b;
			break;
		case '/':
			result = a/b;
			break;
	}
	return result;
}


double calculation (char *folmualr)
{
	opstack  op_stack;
	numstack  num_stack;
	
	int value = 0;
	uint8_t take_num = 0;
	
	init_numstack(&num_stack);
	init_opstack(&op_stack);
	
	for ( int i = 0; folmualr[i] != '\0'; i++)
	{	
		char key = folmualr[i];
		
		//숫자이면 value에 자릿수 계산해서 넣기
		while ( key >= '0' && key <='9')
		{
			value = value * 10 + (key - '0');
			take_num =1; // 숫자 가져오는 중
			break;
		}
		
		// 여는 괄호가 나오면 연산 스택에 넣기
		if (key == '(')
		{
			op_push(&op_stack,key);
		}
		
		// 여는 괄호가 나올 때 까지 괄호 안 계산
		else if (key == ')')
		{
			if (take_num == 1) //숫자를 쓰는 중이었으면 숫자 스택에 넣기
			{
				num_push(&num_stack, value); 
				value =0;
				take_num=0;
			}
			if (!op_empty(&op_stack)&& op_peep(&op_stack) != '(')
			{
				num_push(&num_stack,one_cal(&num_stack,&op_stack)); //계산한 결과 숫자 스택에 넣기
			}
			if (!op_empty(&op_stack)&& op_peep(&op_stack) == '(')
			{
				op_pop(&op_stack); // 여는 괄호 버리기
			}
		}
		
		// 연산자 
		else if (key=='+' || key=='-' || key == '*' || key == '/')
		{
			if (take_num == 1)
			{
				num_push(&num_stack,value);
				value = 0;
				take_num = 0;
			}
			
			while (!op_empty(&op_stack) && priority(op_peep(&op_stack)) >= priority(key)) // 우선순위 비교해서 스택에 든 연산자가 높으면 먼저 계산
			{
				num_push(&num_stack,one_cal(&num_stack,&op_stack)); //계산한 결과 숫자 스택에 넣기
			}
			op_push(&op_stack,key); // 남은 연산자 스택에 넣기
		}
	}
		
		// 남은 연산 전부 게산해서 결과값 리턴
		
		if (take_num == 1)
		{
			num_push(&num_stack,value);
			value = 0;
			take_num = 0;
		}
				
		while (!op_empty(&op_stack))
		{
			num_push(&num_stack,one_cal(&num_stack,&op_stack)); //계산한 결과 숫자 스택에 넣기
		}
		 return num_pop(&num_stack); // 결과 값 리턴
}

