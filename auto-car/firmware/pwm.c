/*
 * pwm.c
 *  - 방향 비트는 PORTF 하위 니블(PF0~PF3)만 갱신한다.
 *    상위 니블(FND2 자릿수)은 1ms FND 인터럽트가 사용하므로,
 *    방향 변경은 인터럽트를 잠깐 막아 원자적으로 처리한다.
 *  - 표시(FND)는 fnd_set_state() 를 통해서만 갱신한다.
 */
#include "pwm.h"
#include "fnd.h"
#include <avr/interrupt.h>

/* PF0~PF3 방향 비트만 원자적으로 설정 (FND2 자릿수 보존) */
static void set_direction(uint8_t dir_bits)
{
    uint8_t sreg = SREG;
    cli();
    MOTOR_DIRECTION_PORT =
        (MOTOR_DIRECTION_PORT & ~MOTOR_DIR_MASK) | (dir_bits & MOTOR_DIR_MASK);
    SREG = sreg;
}

void init_timer1_pwm(void)
{
    /* 분주비 64 -> 250kHz (4us/tick) */
    TCCR1B |= (1 << CS11) | (1 << CS10);

    /* 모드 14 : 고속 PWM, TOP = ICR1 */
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM13) | (1 << WGM12);

    /* 비반전 : 비교일치 시 LOW, BOTTOM 에서 HIGH */
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1);

    ICR1  = 0x3FF;   /* TOP = 1023 -> 약 4ms 주기 */
    OCR1A = 0;
    OCR1B = 0;
}

void init_motor_driver(void)
{
    /* PWM 출력핀 */
    MOTOR_PWM_DDR |= (1 << MOTOR_LEFT_PWM_PIN) | (1 << MOTOR_RIGHT_PWM_PIN);
    /* 방향핀 PF0~PF3 출력 */
    MOTOR_DIRECTION_DDR |= MOTOR_DIR_MASK;
    set_direction(0x05);   /* 전진 방향(IN1,IN3) 초기화 */
}

void init_motor(void)
{
    init_motor_driver();
    init_timer1_pwm();
    stop();
}

/* IN1=PF0, IN2=PF1, IN3=PF2, IN4=PF3 */
void forward(int speed)
{
    set_direction(0x05);          /* IN1,IN3 = 1 */
    OCR1A = OCR1B = speed;
    fnd_set_state(DRIVE_FORWARD);
}

void backward(int speed)
{
    set_direction(0x0A);          /* IN2,IN4 = 1 */
    OCR1A = OCR1B = speed;
    fnd_set_state(DRIVE_BACKWARD);
}

void turn_left(int speed)
{
    set_direction(0x05);          /* 전진 방향 유지 */
    OCR1A = 0;                    /* 왼쪽 정지 */
    OCR1B = speed;               /* 오른쪽만 구동 */
    fnd_set_state(DRIVE_LEFT);
}

void turn_right(int speed)
{
    set_direction(0x05);
    OCR1A = speed;               /* 왼쪽만 구동 */
    OCR1B = 0;                    /* 오른쪽 정지 */
    fnd_set_state(DRIVE_RIGHT);
}

void stop(void)
{
    set_direction(0x0F);          /* 모든 IN = 1 : 브레이크 */
    OCR1A = OCR1B = 0;
    fnd_set_state(DRIVE_STOP);
}
