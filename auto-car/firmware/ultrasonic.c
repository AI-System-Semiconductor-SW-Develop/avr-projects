#include "ultrasonic.h"
#include <util/atomic.h>

#define MEASURE_WAITING  0
#define MEASURE_DONE     1
#define MEASURE_NO_ECHO  2
#define MEASURE_TOO_FAR  3

volatile uint16_t left_distance = 0;
volatile uint16_t center_distance = 0;
volatile uint16_t right_distance = 0;

volatile uint8_t active_sensor = SENSOR_NONE;
volatile uint8_t rising_edge_received = 0;
volatile uint8_t ultrasonic_done = 0;

/* 왼쪽 센서: PE4, INT4 */
ISR(INT4_vect)
{
    if (active_sensor != SENSOR_LEFT)
        return;

    /* ECHO가 HIGH이면 상승 엣지 */
    if (PINE & (1 << ECHO_LEFT))
    {
        TCNT3 = 0;
        rising_edge_received = 1;
    }
    /* ECHO가 LOW이면 하강 엣지 */
    else if (rising_edge_received)
    {
        /*
         * 분주비 64에서 한 카운트는 4us
         * 거리 = TCNT3 * 4 / 58
         *      = TCNT3 * 2 / 29
         */
        left_distance = ((uint32_t)TCNT3 * 2) / 29;

        rising_edge_received = 0;
        active_sensor = SENSOR_NONE;
        ultrasonic_done = 1;
    }
}


/* 중앙 센서: PE5, INT5 */
ISR(INT5_vect)
{
    if (active_sensor != SENSOR_CENTER)
        return;

    if (PINE & (1 << ECHO_CENTER))
    {
        TCNT3 = 0;
        rising_edge_received = 1;
    }
    else if (rising_edge_received)
    {
        center_distance = ((uint32_t)TCNT3 * 2) / 29;

        rising_edge_received = 0;
        active_sensor = SENSOR_NONE;
        ultrasonic_done = 1;
    }
}


/* 오른쪽 센서: PE6, INT6 */
ISR(INT6_vect)
{
    if (active_sensor != SENSOR_RIGHT)
        return;

    if (PINE & (1 << ECHO_RIGHT))
    {
        TCNT3 = 0;
        rising_edge_received = 1;
    }
    else if (rising_edge_received)
    {
        right_distance = ((uint32_t)TCNT3 * 2) / 29;

        rising_edge_received = 0;
        active_sensor = SENSOR_NONE;
        ultrasonic_done = 1;
    }
}


/* 초음파 초기화 */
void init_ultrasonic(void)
{
    /* PA0~PA2 TRIG 출력 */
    DDRA |= (1 << TRIG_LEFT)
        | (1 << TRIG_CENTER)
        | (1 << TRIG_RIGHT);

    PORTA &= ~((1 << TRIG_LEFT)
        | (1 << TRIG_CENTER)
        | (1 << TRIG_RIGHT));

    /* PE4~PE6 ECHO 입력 */
    DDRE &= ~((1 << ECHO_LEFT)
        | (1 << ECHO_CENTER)
        | (1 << ECHO_RIGHT));

    EICRB &= ~((1 << ISC41) | (1 << ISC40)
        | (1 << ISC51) | (1 << ISC50)
        | (1 << ISC61) | (1 << ISC60));

    EICRB |= (1 << ISC40)
        | (1 << ISC50)
        | (1 << ISC60);

    /* 기존 인터럽트 플래그 제거 */
    EIFR = (1 << INTF4)
        | (1 << INTF5)
        | (1 << INTF6);

    /* INT4, INT5, INT6 인터럽트 허용 */
    EIMSK |= (1 << INT4)
        | (1 << INT5)
        | (1 << INT6);

    TCCR3A = 0;
    TCCR3B = (1 << CS31) | (1 << CS30);
    TCNT3 = 0;
}


/*
 * 초음파 센서 한 개 측정
 *
 *  반환값
 *    0               : 센서 실패 (Trigger 후 Echo 상승이 오지 않음)
 *    1 ~ DIST_FAR_CM : 측정 거리(cm)
 *    DIST_FAR_CM     : Echo는 올라왔지만 ECHO_TIMEOUT 안에 내려오지 않음 = 측정 범위 밖(멀다)
 *
 *  최악 대기 시간: RISE_TIMEOUT(5ms) + ECHO_TIMEOUT(30ms) = 약 35ms
 *    (Echo 상승 시 ISR이 TCNT3를 0으로 되돌리므로 두 구간이 더해진다)
 *
 *  TCNT3는 16비트 레지스터라 상위 바이트를 임시 레지스터(TEMP)에 담아 두 번에 나눠 읽는다.
 *  메인이 읽는 도중 Echo ISR이 TCNT3에 접근하면 TEMP가 덮여 값이 깨지므로,
 *  메인에서의 TCNT3 접근과 상태 판단은 모두 ATOMIC_BLOCK 안에서 한다.
 */
uint16_t measure_sensor(uint8_t sensor, uint8_t trig_pin)
{
    uint8_t result_state = MEASURE_WAITING;
    uint16_t result = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ultrasonic_done = 0;
        rising_edge_received = 0;

        /* 해당 센서의 이전 인터럽트 플래그 제거 */
        if (sensor == SENSOR_LEFT)
            EIFR = (1 << INTF4);
        else if (sensor == SENSOR_CENTER)
            EIFR = (1 << INTF5);
        else
            EIFR = (1 << INTF6);

        TCNT3 = 0;
        active_sensor = sensor;
    }

    /* 10us TRIG 신호 발생 */
    PORTA &= ~(1 << trig_pin);
    _delay_us(2);
    PORTA |= (1 << trig_pin);
    _delay_us(10);
    PORTA &= ~(1 << trig_pin);

    /* ISR이 측정을 끝내거나 시간 초과가 날 때까지 대기 */
    while (result_state == MEASURE_WAITING)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            uint16_t count = TCNT3;      /* 인터럽트 차단 중 읽기 → TEMP 충돌 없음 */

            if (ultrasonic_done)
            {
                result_state = MEASURE_DONE;
            }
            else if (!rising_edge_received && count >= RISE_TIMEOUT_COUNT)
            {
                active_sensor = SENSOR_NONE;     /* 늦게 온 Echo는 무시 */
                result_state = MEASURE_NO_ECHO;
            }
            else if (rising_edge_received && count >= ECHO_TIMEOUT_COUNT)
            {
                active_sensor = SENSOR_NONE;
                rising_edge_received = 0;
                result_state = MEASURE_TOO_FAR;
            }
        }
    }

    if (result_state == MEASURE_DONE)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (sensor == SENSOR_LEFT)
                result = left_distance;
            else if (sensor == SENSOR_CENTER)
                result = center_distance;
            else
                result = right_distance;
        }
        if (result == 0)
            result = 1;                  /* 0은 실패 전용 값으로 남겨 둔다 */
        if (result > DIST_FAR_CM)
            result = DIST_FAR_CM;
    }
    else if (result_state == MEASURE_TOO_FAR)
    {
        result = DIST_FAR_CM;
    }
    else
    {
        result = 0;                      /* MEASURE_NO_ECHO: 센서 실패 */
    }

    return result;
}


/* 초음파 간섭 방지를 위해 차례대로 측정 */
void read_all_sensor(void)
{
    left_distance = measure_sensor(SENSOR_LEFT, TRIG_LEFT);
    _delay_ms(20);

    center_distance = measure_sensor(SENSOR_CENTER, TRIG_CENTER);
    _delay_ms(20);

    right_distance = measure_sensor(SENSOR_RIGHT, TRIG_RIGHT);
    _delay_ms(20);
}
