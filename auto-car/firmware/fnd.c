/*
 * fnd.c
 *
 * Created: 2026-06-12 오전 10:52:30
 *  Author: kccistc
 */ 

#include "fnd.h"
#include <avr/interrupt.h>
#include <util/atomic.h>


#define BLANK  0xff
#define DASH   0xbf

#define FONT_F 0x8e
#define FONT_L 0xc7
#define FONT_R 0xaf
#define FONT_B 0x83
#define FONT_S 0x92
#define FONT_T 0x87
#define FONT_O 0xc0
#define FONT_P 0x8c


static const uint8_t fnd_font[10] =
{
    0xc0, 0xf9, 0xa4, 0xb0, 0x99,
    0x92, 0x82, 0xf8, 0x80, 0x90
};

volatile uint8_t fnd_buffer[8];

volatile uint16_t drive_seconds = 0;
volatile uint16_t backward_count = 0;
volatile uint16_t left_count = 0;
volatile uint16_t right_count = 0;

volatile uint8_t drive_finished = 0;
volatile uint8_t drive_running = 1;
volatile uint8_t result_page = 0;
volatile uint8_t animation_position = 0;

volatile drive_state_t current_state = DRIVE_STOP;


/* 1ms마다 FND 자릿수 변경 및 시간 계산 */
ISR(TIMER2_OVF_vect)
{
    static uint8_t digit = 0;
    static uint16_t ms_count = 0;
    static uint16_t animation_count = 0;

    TCNT2 = 6;

    /* 모든 자릿수 끄기 */
    FND1_DIGIT_PORT |= 0xf0;
    FND2_DIGIT_PORT |= 0xf0;

    /* 세그먼트 출력 */
    FND_DATA_PORT = fnd_buffer[digit];

    /* 한 자리만 켜기 */
    if (digit < 4)
        FND1_DIGIT_PORT &= ~(1 << (digit + 4));
    else
        FND2_DIGIT_PORT &= ~(1 << digit);

    digit = (digit + 1) % 8;

    if (drive_running)
    {
        ms_count++;

        if (ms_count >= 1000)
        {
            ms_count = 0;
            drive_seconds++;
        }

        animation_count++;

        if (animation_count >= 150)
        {
            animation_count = 0;
            animation_position = (animation_position + 1) % 3;
        }
    }
}


void init_fnd(void)
{
    FND_DATA_DDR = 0xff;

    /* PD4~PD7 */
    FND1_DIGIT_DDR |= 0xf0;

    /* PF4~PF7, PF0~PF3은 모터 */
    FND2_DIGIT_DDR |= 0xf0;

    FND_DATA_PORT = 0xff;
    FND1_DIGIT_PORT |= 0xf0;
    FND2_DIGIT_PORT |= 0xf0;

    for (uint8_t i = 0; i < 8; i++)
        fnd_buffer[i] = BLANK;

    drive_seconds = 0;
    backward_count = 0;
    left_count = 0;
    right_count = 0;

    drive_finished = 0;
    drive_running = 1;
    result_page = 0;
    current_state = DRIVE_STOP;

    /*
     * Timer2 일반 모드, 분주비 64
     * 16MHz / 64 = 250kHz
     * 250카운트 = 1ms
     */
    TCCR2 = (1 << CS22);
    TCNT2 = 6;
    TIMSK |= (1 << TOIE2);
}


/* 모터 동작 상태와 횟수를 함께 처리 */
void fnd_set_state(drive_state_t state)
{
    /* 주행 종료 */
    if (state == DRIVE_END)
    {
        drive_running = 0;
        drive_finished = 1;
        current_state = DRIVE_STOP;
        result_page = 0;
        return;
    }

    /* 같은 동작을 반복 호출하면 횟수를 올리지 않음 */
    if (state == current_state)
        return;

    current_state = state;

    if (!drive_running)
        return;

    if (state == DRIVE_BACKWARD)
        backward_count++;

    else if (state == DRIVE_LEFT)
        left_count++;

    else if (state == DRIVE_RIGHT)
        right_count++;
}


/* 종료 후 결과 화면 변경 */
void fnd_next_page(void)
{
    if (drive_finished)
        result_page = (result_page + 1) % 4;
}


/* main의 while문에서 반복 호출 */
void fnd_processing(void)
{
    uint16_t minute;
    uint8_t second;
    uint16_t value = 0;
    uint16_t seconds;
    uint8_t buf[8];          /* 지역 버퍼에서 완성한 뒤 한 번에 복사 (ISR이 빈 화면을 출력하지 않도록) */

    /* drive_seconds는 1ms ISR이 갱신하는 16비트 값 → 두 바이트를 한 번에 읽는다 */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        seconds = drive_seconds;
    }

    for (uint8_t i = 0; i < 8; i++)
        buf[i] = BLANK;

    /* 주행 중 */
    if (!drive_finished)
    {
        minute = seconds / 60;
        second = seconds % 60;

        if (minute > 99)
            minute = 99;

        /* 앞 4자리: MMSS */
        buf[0] = fnd_font[minute / 10];
        buf[1] = fnd_font[minute % 10];
        buf[2] = fnd_font[second / 10];
        buf[3] = fnd_font[second % 10];

        switch (current_state)
        {
        case DRIVE_FORWARD:
            buf[4] = FONT_F;
            buf[5 + animation_position] = DASH;
            break;

        case DRIVE_BACKWARD:
            buf[4] = FONT_B;
            buf[7 - animation_position] = DASH;
            break;

        case DRIVE_LEFT:
            buf[4] = FONT_L;
            buf[7 - animation_position] = DASH;
            break;

        case DRIVE_RIGHT:
            buf[4] = FONT_R;
            buf[5 + animation_position] = DASH;
            break;

        case DRIVE_STOP:
        default:
            buf[4] = FONT_S;
            buf[5] = FONT_T;
            buf[6] = FONT_O;
            buf[7] = FONT_P;
            break;
        }
    }

    /* 주행 종료 후 */
    else
    {
        switch (result_page)
        {
        case 0:     // 주행시간
            minute = seconds / 60;
            second = seconds % 60;

            if (minute > 99)
                minute = 99;

            buf[0] = FONT_T;

            buf[4] = fnd_font[minute / 10];
            buf[5] = fnd_font[minute % 10];
            buf[6] = fnd_font[second / 10];
            buf[7] = fnd_font[second % 10];
            break;

        case 1:     // 후진 횟수
            buf[0] = FONT_B;
            value = backward_count;
            break;

        case 2:     // 좌회전 횟수
            buf[0] = FONT_L;
            value = left_count;
            break;

        case 3:     // 우회전 횟수
            buf[0] = FONT_R;
            value = right_count;
            break;
        }

        if (result_page != 0)
        {
            if (value > 9999)
                value = 9999;

            buf[4] = fnd_font[value / 1000];
            buf[5] = fnd_font[value / 100 % 10];
            buf[6] = fnd_font[value / 10 % 10];
            buf[7] = fnd_font[value % 10];
        }
    }

    /* 완성된 화면을 표시 버퍼로 복사 (바이트 단위 쓰기는 원자적) */
    for (uint8_t i = 0; i < 8; i++)
        fnd_buffer[i] = buf[i];
}
