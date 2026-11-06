/*
 * main.c  -  06.AUTO_CAR
 *
 *  - 수동 모드 : 블루투스 명령(F/B/L/R/S)으로 조종, E=주행종료
 *  - 자동 모드 : 초음파 3개로 장애물 회피 주행
 *  - 버튼(PB4) : 주행 중에는 수동/자동 토글, 주행 종료 후에는 결과 페이지 전환
 *  - FND       : 주행시간(MMSS)+상태 / 종료 후 시간·후진·좌회전·우회전 횟수
 */
#include "define.h"
#include "button.h"
#include "pwm.h"
#include "ultrasonic.h"
#include "fnd.h"
#include "uart0.h"
#include "uart1.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

FILE OUTPUT = FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_WRITE);

static uint8_t mode = MANUAL_MODE;

static void execute_command(uint8_t command);


/* 블루투스 수동 조종: 큐에 쌓인 명령을 들어온 순서대로 모두 처리 */
static void manual_step(void)
{
    uint8_t command;

    while ((command = bt_read()) != 0)
    {
        UART0_transmit(command);     /* 확인용 에코 (ISR이 아닌 메인에서) */
        execute_command(command);

        if (drive_finished)          /* E 처리 후 남은 명령은 버림 */
            break;
    }
}

static void execute_command(uint8_t command)
{
    switch (command)
    {
    case 'F': case 'f': forward(500);  break;
    case 'B': case 'b': backward(600); break;
    case 'L': case 'l': turn_left(700);  break;
    case 'R': case 'r': turn_right(700); break;
    case 'S': case 's': stop();        break;
    case 'E': case 'e': stop(); fnd_set_state(DRIVE_END); break;
    default: break;          /* \r, \n 등 그 외 문자는 무시 */
    }
}


/* 초음파 장애물 회피 자동 주행 */
static void auto_step(void)
{
    read_all_sensor();

    /* 0 = 센서 실패(Echo 없음), DIST_FAR_CM = 범위 밖(멀다) */

    /* 중앙 센서 측정 실패 -> 안전 정지 */
    if (center_distance == 0)
    {
        stop();
        return;
    }

    /* 매우 가까움 -> 후진 */
    if (center_distance < DIST_VERY_NEAR)
    {
        backward(150);
        return;
    }

    /* 가까움 -> 넓은 쪽으로 회전 */
    if (center_distance < DIST_NEAR)
    {
        if (left_distance == 0 && right_distance == 0)
            stop();
        else if (left_distance > right_distance)
            turn_left(160);
        else
            turn_right(160);
        return;
    }

    /* 앞이 비어 있음 -> 전진 */
    forward(180);
}


int main(void)
{
    init_button();
    init_uart0();
    init_uart1();
    init_fnd();        
    init_motor();
    init_ultrasonic();

    stdout = &OUTPUT;

    sei();

    stop();
    _delay_ms(1000);

    while (1)
    {
        /* 버튼 처리 */
        if (get_button(BUTTON0, BUTTON0PIN))
        {
            if (drive_finished)
            {
                fnd_next_page();              /* 결과 페이지 전환 */
            }
            else
            {
                mode = (mode == MANUAL_MODE) ? AUTO_MODE : MANUAL_MODE;
                stop();
                bt_flush();                   /* 자동 모드 중 쌓인 명령이 전환 직후 실행되지 않도록 */
            }
        }

        /* 주행 */
        if (!drive_finished)
        {
            if (mode == AUTO_MODE)
                auto_step();
            else
                manual_step();
        }

        fnd_processing();
    }
}
