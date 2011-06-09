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

#include "audio.h"
#include "led.h"

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
    {76, 4}, {71, 2}, {72, 2}, {74, 2}, {76, 1}, {74, 1}, {72, 2}, {71, 2},
    {69, 2}, {64, 2}, {69, 2}, {72, 2}, {76, 4}, {74, 2}, {72, 2},
    {71, 4}, {71, 2}, {72, 2}, {74, 4}, {76, 4},
    {72, 4}, {69, 4}, {69, 8},
    {74, 2}, {69, 2}, {74, 2}, {77, 2}, {81, 4}, {79, 2}, {77, 2},
    {76, 6}, {72, 2}, {76, 2}, {77, 1}, {76, 1}, {74, 2}, {72, 2},
    {71, 4}, {71, 2}, {72, 2}, {74, 4}, {76, 4},
    {72, 4}, {69, 4}, {69, 8},
    {69, 2}, {64, 2}, {60, 2}, {64, 2}, {69, 2}, {64, 2}, {60, 2}, {64, 2},
    {68, 2}, {64, 2}, {59, 2}, {64, 2}, {68, 2}, {64, 2}, {59, 2}, {64, 2},
    {67, 2}, {62, 2}, {59, 2}, {62, 2}, {66, 2}, {60, 2}, {65, 2}, {60, 2},
    {64, 2}, {59, 2}, {56, 2}, {59, 2}, {64, 2}, {59, 2}, {56, 2}, {59, 2},
    {69, 2}, {64, 2}, {60, 2}, {64, 2}, {69, 2}, {64, 2}, {60, 2}, {64, 2},
    {71, 2}, {68, 2}, {64, 2}, {68, 2}, {71, 2}, {68, 2}, {64, 2}, {68, 2},
    {72, 2}, {67, 2}, {73, 2}, {69, 2}, {74, 2}, {69, 2}, {75, 2}, {70, 2},
    {76, 1}, {77, 1}, {76, 1}, {75, 1}, {76, 1}, {77, 1}, {76, 1}, {75, 1}, {76, 8},
};
#define ROLL_SIZE (sizeof(roll)/sizeof(roll[0]))

int main() {
    unsigned char key_idx = ROLL_SIZE, duration = 0;
    struct note *note = roll;
    /* Step, in cycles; multiply ms by 256. 250 is the maximum here. */
    unsigned short step = 100 * 256;

    initialize();

    start_scanout();
    start_audio();

    while (1) {
        /* Mute, if switch 7 is set. */
        enable_audio(PINA & _BV(PA7));

        /* Check timer to advance the roll. */
        if (!duration) {
            key_idx++;
            if (key_idx >= ROLL_SIZE) {
                key_idx = 0;
            }
            note = roll + key_idx;
            set_audio(note->pitch);
            duration = note->duration;
        }

        duration--;
        _delay_loop_2(step);
    }
}
