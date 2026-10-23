/*
 * calculator.h
 *
 * Created: 2026-07-01 오전 9:42:34
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <ctype.h>
#include "keypad.h"

#define STACK_SIZE 100


typedef struct 
{
	char op_data[STACK_SIZE];
	int top;
}opstack;

typedef struct  
{
	double num_data[STACK_SIZE];
	int top;
}numstack;

double calculation (char *folmualr);
