#ifndef AUTO_CAR_FND_H
#define AUTO_CAR_FND_H

/*
 * fnd.h
 *
 * Created: 2026-06-12 오전 10:46:01
 *  Author: user
 */ 

#include <avr/io.h>
#include <stdint.h>

#define FND_DATA_DDR       DDRC
#define FND_DATA_PORT      PORTC

#define FND1_DIGIT_DDR     DDRD
#define FND1_DIGIT_PORT    PORTD

#define FND2_DIGIT_DDR     DDRF
#define FND2_DIGIT_PORT    PORTF

typedef enum
{
    DRIVE_STOP,
    DRIVE_FORWARD,
    DRIVE_BACKWARD,
    DRIVE_LEFT,
    DRIVE_RIGHT,
    DRIVE_END
} drive_state_t;

extern volatile uint8_t drive_finished;

void init_fnd(void);
void fnd_set_state(drive_state_t state);
void fnd_processing(void);
void fnd_next_page(void);

#endif /* AUTO_CAR_FND_H */
