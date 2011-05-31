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
    DDRB=0b11001111;

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

static unsigned char keys[] = {
    //60, 60, 62, 64, 71, 71, 69, 69, 60, 60, 62, 64, 67, 65, 60, 62
    72, 72, 74, 76, 83, 83, 81, 81, 72, 72, 74, 76, 79, 77, 72, 74
};

/* Set the audio out to a certain amplitude.
 * Call this a lot, at regular intervals, to make sounds. */
int set_audio(unsigned char amp) {
    /* Mask to four bits. */
    amp &= 0xf;

    /* Shift into the register. */
    PORTB &= 0xc3;
    PORTB |= amp << 2;
}

/** Main Function */

int main(void) {
    /** Local Varibles */
    unsigned char temp = 0, key_idx = 0, key, amplitude;
    unsigned short delay;

    initialize();
    clearArray();

    /* Set the timer for CTC mode. */
    TCCR3A |= _BV(WGM31);

    /* Start timer, no prescaling. */
    TCCR3B |= _BV(CS30);

    /* Set the initial delay. */
    OCR3A = key_us[61] >> 2;

    /* Set the timer for CTC mode. */
    TCCR1A |= _BV(WGM11);

    /* Start timer, prescaled to 1024. */
    TCCR1B |= _BV(CS12) | _BV(CS10);

    /* Set the delay, permanently. */
    OCR1A = 1024;

    /* Set the color of LEDs, for debugging with PORTC. */
    PORTB = 0x80;

    while (1) {
        /* Check timer to advance the roll. */
        if (TIFR1 & _BV(OCF1A)) {
            TIFR1 = _BV(OCF1A);
            key_idx++;
            if (key_idx >= 16) {
                key_idx = 0;
            }
            key = keys[key_idx];
            //OCR3A = key_us[key] >> 2;
        }

        PORTC = TCNT3H;

        /* Check timer for sound wave. */
        if (TIFR3 & _BV(OCF3A)) {
            TIFR3 = _BV(OCF3A);
            temp++;
            if (temp > 0xf) {
                temp = 0x0;
            }

            /* Put the audio. */
            set_audio(temp);
        }
    }
}//main
