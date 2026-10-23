#include "ultrasonic.h"

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


/* 초음파 센서 한 개 측정 */
uint16_t measure_sensor(uint8_t sensor, uint8_t trig_pin)
{
    uint16_t result;
    uint8_t old_sreg;

    ultrasonic_done = 0;
    rising_edge_received = 0;
    active_sensor = sensor;

    /* 거리값 초기화 */
    if (sensor == SENSOR_LEFT)
        left_distance = 0;
    else if (sensor == SENSOR_CENTER)
        center_distance = 0;
    else
        right_distance = 0;

    /* 해당 센서의 이전 인터럽트 플래그 제거 */
    if (sensor == SENSOR_LEFT)
        EIFR = (1 << INTF4);
    else if (sensor == SENSOR_CENTER)
        EIFR = (1 << INTF5);
    else
        EIFR = (1 << INTF6);

    TCNT3 = 0;

    /* 10us TRIG 신호 발생 */
    PORTA &= ~(1 << trig_pin);
    _delay_us(2);

    PORTA |= (1 << trig_pin);
    _delay_us(10);

    PORTA &= ~(1 << trig_pin);

    /* ISR에서 측정 완료할 때까지 기다림 */
    while (!ultrasonic_done)
    {
        /*
         * 약 30ms 타임아웃
         */
        if (TCNT3 >= TIMEOUT_COUNT)
        {
            old_sreg = SREG;
            cli();

            if (!ultrasonic_done)
            {
                active_sensor = SENSOR_NONE;
                rising_edge_received = 0;

                SREG = old_sreg;
                return 0;
            }

            SREG = old_sreg;
        }
    }

    if (sensor == SENSOR_LEFT)
        result = left_distance;
    else if (sensor == SENSOR_CENTER)
        result = center_distance;
    else
        result = right_distance;

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
