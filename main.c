/**
  @file main.c
  @brief Lab 6 Starter Code
  @version .01 
  @mainpage Lab 6 Starter Code

  @section intro Code Overview
  This first lab allows outputs to the LED array and single patteren of lights. You need to revise it so that it outputs lights based on the input from the switches.

  @section hw Hardware Pin Out
PORTA:
Switches A7 - A0

PORTB:



*/

/** Constants */
#define F_CPU 1000000U

/** Includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "keys.h"

/** Global Variables */

/** Functions */

/** The initialize() function initializes all of the Data Direction Registers for the Wunderboard. Before making changes to DDRx registers, ensure that you have read the peripherals section of the Wunderboard user guide.*/
void initialize (void) {
    /** Port A is the switches and buttons. They should always be inputs. ( 0 = Input and 1 = Output )*/
    DDRA=0b00000000;

    /** Port B has the LED Array color control, SD card, and audio-out on it. Leave DDRB alone. ( 0 = Input and 1 = Output )*/
    DDRB=0b11110000;

    /** Port C is for the 'row' of the LED array. They should always be outputs. ( 0 = Input and 1 = Output )*/
    DDRC=0b11111111;

    /** Port D has the Serial on it. Leave DDRB alone. ( 0 = Input and 1 = Output )*/
    DDRD=0b00000000;

    /** Port E has the LED Array Column control out on it. Leave DDRE alone. ( 0 = Input and 1 = Output )*/
    DDRE=0b00000111;

    /** Port F has the accelerometer and audio-in on it. Leave DDRF alone. ( 0 = Input and 1 = Output )*/
    DDRF=0b00000000;
}

void clearArray(void)
{
    PORTB &= ~((1 << PB6) | (1 << PB7));	// Disable latches
    PORTC = 0x00;
    PORTB |= (1 << PB6) | (1 << PB7);		// Enable latches
    PORTB &= ~((1 << PB6) | (1 << PB7));	// Disable latches
}

struct note {
    unsigned char pitch;
    unsigned char duration;
};

static struct note roll[] = {
    {76, 4}, {71, 2}, {72, 2}, {74, 2}, {76, 1}, {74, 1}, {72, 2}, {71, 2},
    {69, 2}, {64, 2}, {69, 2}, {72, 2}, {76, 4}, {74, 2}, {72, 2},
    {71, 4}, {71, 2}, {72, 2}, {74, 4}, {76, 4},
    {72, 4}, {69, 4}, {69, 8},
    {74, 2}, {69, 2}, {74, 2}, {77, 2}, {81, 4}, {79, 2}, {77, 2},
    {76, 6}, {72, 2}, {76, 2}, {77, 1}, {76, 1}, {74, 2}, {72, 2},
    {71, 4}, {71, 2}, {72, 2}, {74, 4}, {76, 4},
    {72, 4}, {69, 4}, {69, 8},
};
#define ROLL_SIZE (sizeof(roll)/sizeof(roll[0]))

/* Set the pitch of the audio out. */
void set_audio(unsigned char pitch) {
    unsigned short lambda;

    /* Look up the wavelength in the table. */
    lambda = key_us[pitch];

    /* Halve it, because we only represent half the wave here. */
    lambda >>= 1;

    /* Put the new value into the register. */
    OCR1A = lambda;

    /* And reset the timer. This gives a very small breath mark/attack to the
     * note. */
    TCNT1 = 0x0;
}

static unsigned char red_plane[8] = {
    0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char green_plane[8] = {
    0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Start the scanout timer and interrupt.
 * The scanout timer is always timer0.
 * Every 3ms, the timer will fire an interrupt. An ISR for that interrupt
 * could update one row of the LED array every time, and still refresh the
 * entire array in 24ms, equivalent to ~40fps, flicker-free. */
void start_scanout() {
    /* Enable CTC mode. */
    TCCR0A = _BV(WGM01);

    /* Start timer for scanout: prescale 1024. */
    TCCR0B = _BV(CS02) | _BV(CS00);

    /* Set the timer to 3 ticks. */
    TCNT0 = 0x0;
    OCR0A = 0x3;

    /* Enable the interrupt for this timer. */
    TIMSK0 |= _BV(OCIE0A);

    /* Enable all interrupts, if not already done. */
    sei();
}

/* ISR for timer0. Updates the LED array. */
ISR(TIMER0_COMPA_vect) {
    unsigned char row;

    /* Shift to the next row in the array. */
    row = PORTE & 0x7;
    row++;
    PORTE = row & 0x7;

    /* Clear row. */
    PORTB |= _BV(PB7) | _BV(PB6);
    PORTC = 0x0;

    if (row == 0) {
        return;
    }

    /* Draw greens. */
    PORTB |= _BV(PB7);
    PORTB &= ~_BV(PB6);
    PORTC = green_plane[row];

    /* Draw reds. */
    PORTB |= _BV(PB6);
    PORTB &= ~_BV(PB7);
    PORTC = red_plane[row];
}

/** Main Function */

int main(void) {
    /** Local Varibles */
    unsigned char temp = 0, key_idx = ROLL_SIZE, duration = 0;
    struct note *note = roll;
    unsigned short delay = 65535;

    initialize();
    clearArray();

    start_scanout();

    /* Enable audio out. */
    PORTB |= _BV(PB5);

    /* Start timer for audio: CTC, toggle on match, no prescaling. */
    TCCR1A = _BV(COM1A0);
    TCCR1B = _BV(WGM12) | _BV(CS10);

    /* Set the delay for the first note. */
    OCR3A = 10;

    while (1) {
        /* If that switch is flipped, mute. */
        if (PINA & _BV(PA7)) {
            PORTB &= ~_BV(PB5);
        } else {
            PORTB |= _BV(PB5);
        }

        /* Check timer to advance the roll. */
        if (!duration) {
            key_idx++;
            if (key_idx >= ROLL_SIZE) {
                key_idx = 0;
            }
            note = roll + key_idx;
            set_audio(note->pitch);
            green_plane[7] = note->pitch;
            duration = note->duration;
        }

        duration--;
        _delay_loop_2(delay);
        delay -= 50;
    }
}//main
