/* LED array handler.
 * This file provides an interrupt-driven, asynchronous, automatic array
 * updater. */

#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#include "led.h"

unsigned char red_plane[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

unsigned char green_plane[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

struct sprite sprite0;

/* Start the scanout timer and interrupt.
 * The scanout timer is always timer0.
 * Every 1ms, the timer will fire an interrupt. An ISR for that interrupt
 * could update one row of the LED array every time. */
void start_scanout() {
    /* Enable CTC mode. */
    TCCR0A = _BV(WGM01);

    /* Start timer for scanout: prescale 1024. */
    TCCR0B = _BV(CS02) | _BV(CS00);

    /* Set the timer to 1 tick. */
    TCNT0 = 0x0;
    OCR0A = 0x1;

    /* Enable the interrupt for this timer. */
    TIMSK0 |= _BV(OCIE0A);

    /* Enable all interrupts, if not already done. */
    sei();
}

static unsigned char row = 0;

/* ISR for timer0. Updates the LED array. */
ISR(TIMER0_COMPA_vect) {
    unsigned char temp;

    /* Shift to the next row in the array. */
    row++;
    if (row >= 8) {
        row = 0;
    }
    PORTE &= ~0x7;
    PORTE |= row;

    /* Clear row. */
    PORTB |= _BV(PB7) | _BV(PB6);
    PORTC = 0x0;

    /* Draw greens. */
    PORTB |= _BV(PB7);
    PORTB &= ~_BV(PB6);
    temp = green_plane[row];
    if (row >= sprite0.x) {
        temp |= sprite0.green[row - sprite0.x] >> sprite0.y;
    }
    PORTC = temp;

    /* Draw reds. */
    PORTB |= _BV(PB6);
    PORTB &= ~_BV(PB7);
    temp = red_plane[row];
    if (row >= sprite0.x) {
        temp |= sprite0.red[row - sprite0.x] >> sprite0.y;
    }
    PORTC = temp;
}
