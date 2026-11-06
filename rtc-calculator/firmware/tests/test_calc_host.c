/* PC 시험: gcc -std=gnu99 -Istub -I.. test_calc_host.c -lm -o test_calc && ./test_calc
 * AVR와 같게 double을 32비트 float로 바꿔 calculator.c를 그대로 포함한다. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#define double float
#include "../calculator.c"
static int fails=0;
static void ok(const char*e,double want){double r;calc_status_t s=calculation(e,&r);
 if(s!=CALC_OK||fabs(r-want)>1e-6){printf("FAIL %-16s -> status %d val %g (want %g)\n",e,s,r,want);fails++;} else printf("ok   %-16s = %g\n",e,r);}
static void err(const char*e,calc_status_t want){double r;calc_status_t s=calculation(e,&r);
 if(s!=want){printf("FAIL %-16s -> status %d (want %d)\n",e,s,want);fails++;} else printf("ok   %-16s -> %s\n",e,calc_error_message(s));}
int main(void){
 /* 기술서의 기존 시험 */
 ok("(2+3*4)",14); ok("2*(3+4*5)",46); ok("2+8+(3*9)+(5+5)",47);
 ok("1+2*3",7); ok("10-4-3",3); ok("100/5/2",10); ok("2*3+4*5",26);
 /* 16비트 한계를 넘는 수 */
 ok("40000+1",40001); ok("9999999",9999999); err("12345678",CALC_ERR_RANGE);
 /* 단항 마이너스 */
 ok("-3+5",2); ok("2*-3",-6); ok("-(2+3)",-5); ok("1--2",3); ok("8/-(2)*3",-12); ok("2*-(3+4)",-14);
 ok("((1+2))*3",9); ok("7/2",3.5);
 /* 오류 */
 err("",CALC_ERR_SYNTAX); err("2+",CALC_ERR_SYNTAX); err("*3",CALC_ERR_SYNTAX); err("2+*3",CALC_ERR_SYNTAX);
 err("2(3)",CALC_ERR_SYNTAX); err("(2)3",CALC_ERR_SYNTAX); err("()",CALC_ERR_SYNTAX); err("1---2",CALC_ERR_SYNTAX);
 err("(2+3",CALC_ERR_PAREN); err("2+3)",CALC_ERR_PAREN); err("5/0",CALC_ERR_DIV0); err("5/(2-2)",CALC_ERR_DIV0);
 err("1234567890",CALC_ERR_RANGE); err("9999999*9999999*9999999*9999999*9999999",CALC_ERR_RANGE);
 printf("\n%d failure(s)\n",fails); return fails!=0;}
