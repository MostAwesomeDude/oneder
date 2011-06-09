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

#define UP 0x0
#define DOWN 0x1
#define HELD 0x2

#define FALLING 0x0
#define LANDED 0x1
#define PLACED 0x2

/** Includes */
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/power.h>

#include "adc.h"
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

/* Determine whether sprite0 will collide with anything if moved down one X
 * level. */
unsigned char sprite_will_collide() {
    unsigned char x, row, i;
    x = 7 - sprite0.x;
    /* Off the bottom of the screen. */
    row = sprite0.red[x] | sprite0.green[x];
    if (row) {
        return 1;
    }

    for (i = 0; i < sprite0.h; i++) {
        /* Mask of current row... */
        row = sprite0.red[i] << sprite0.y;
        row |= sprite0.green[i] << sprite0.y;
        /* AND with mask of next row. */
        row &= red_plane[i + sprite0.x + 1] | green_plane[i + sprite0.x + 1];
        if (row) {
            return 1;
        }
    }

    return 0;
}

unsigned char sprite_will_collide_left() {
    unsigned char row, i;

    for (i = 0; i < sprite0.h; i++) {
        /* Mask of current row... */
        row = sprite0.red[i] << sprite0.y;
        row |= sprite0.green[i] << sprite0.y;
        /* Shift left. */
        row >>= 1;
        /* AND with mask of next row. */
        row &= red_plane[i + sprite0.x] | green_plane[i + sprite0.x];
        if (row) {
            return 1;
        }
    }

    return 0;
}

unsigned char sprite_will_collide_right() {
    unsigned char row, i;

    for (i = 0; i < sprite0.h; i++) {
        /* Mask of current row... */
        row = sprite0.red[i] << sprite0.y;
        row |= sprite0.green[i] << sprite0.y;
        /* Shift left. */
        row <<= 1;
        /* AND with mask of next row. */
        row &= red_plane[i + sprite0.x] | green_plane[i + sprite0.x];
        if (row) {
            return 1;
        }
    }

    return 0;
}

void clear_full_lines() {
    unsigned char offset = 0;
    signed char i;

    for (i = 7; i >= 0; i--) {
        if (red_plane[i] == 0xff) {
            /* Full line; clear it. */
            offset++;
        } else if (offset) {
            /* Not a full line? Move it down, if possible. */
            red_plane[i + offset] = red_plane[i];
        }
    }
}

/* Rotate the sprite CCW. */
void rotate_sprite_ccw() {
    unsigned char i, j, temp[8] = { 0 };
    for (i = 0; i < sprite0.h; i++) {
        for (j = 0; j < sprite0.w; j++) {
            if (sprite0.green[i] & _BV(j)) {
                temp[j] |= _BV(i);
            }
        }
    }

    memcpy(sprite0.green, temp, 8);

    i = sprite0.w;
    sprite0.w = sprite0.h;
    sprite0.h = i;
}

void next_piece(struct sprite *s) {
    memset(s->green, 0, 8);
    memset(s->red, 0, 8);

    switch (rand() % 7) {
        /* O block. */
        case 0:
            s->green[0] = 0x3;
            s->green[1] = 0x3;
            s->w = s->h = 2;
            break;
        /* L block. */
        case 1:
            s->green[0] = 0x1;
            s->green[1] = 0x1;
            s->green[2] = 0x3;
            s->w = 2;
            s->h = 3;
            break;
        /* J block. */
        case 2:
            s->green[0] = 0x2;
            s->green[1] = 0x2;
            s->green[2] = 0x3;
            s->w = 2;
            s->h = 3;
            break;
        /* T block. */
        case 3:
            s->green[0] = 0x2;
            s->green[1] = 0x7;
            s->w = 3;
            s->h = 2;
            break;
        /* S block. */
        case 4:
            s->green[0] = 0x6;
            s->green[1] = 0x3;
            s->w = 3;
            s->h = 2;
            break;
        /* Z block. */
        case 5:
            s->green[0] = 0x3;
            s->green[1] = 0x6;
            s->w = 3;
            s->h = 2;
            break;
        /* I block. */
        case 6:
        default:
            s->green[0] = 0xf;
            s->w = 4;
            s->h = 1;
            break;
    }
}

void change_state() {
    static unsigned char state = FALLING;
    unsigned char i;

    switch (state) {
        case FALLING:
            /* Clear red color. */
            memset(sprite0.red, 0, 8);
            if (sprite_will_collide()) {
                /* Next frame, we'll cement it. */
                state = LANDED;
            } else {
                /* No collisions next frame, so move the piece downwards. */
                sprite0.x++;
            }
            break;
        case LANDED:
            /* Add red color to sprite. */
            memcpy(sprite0.red, sprite0.green, 8);
            state = PLACED;
            break;
        case PLACED:
            for (i = 0; i < sprite0.x && i + sprite0.x < 8; i++) {
                red_plane[i + sprite0.x] |= sprite0.green[i] << sprite0.y;
            }
            sprite0.x = 0;
            sprite0.y = 0;
            next_piece(&sprite0);
            state = FALLING;
            break;
    }
}

int main() {
    unsigned char key_idx = ROLL_SIZE, duration = 1, toggle = 0, i;
    unsigned char buttons[4];
    struct note *note = roll;
    unsigned int seed;

    initialize();

    start_scanout();
    start_audio();

    /* Set up timer: CTC only, 1024 prescale. */
    power_timer3_enable();
    TCCR3B = _BV(WGM32) | _BV(CS32) | _BV(CS30);
    OCR3A = 200;
    TCNT3 = 0;

    seed = read_adc(5);
    seed |= read_adc(6) << 8;

    srand(seed);

    next_piece(&sprite0);

    while (1) {
        /* Check buttons. */
        for (i = 0; i < 4; i++) {
            if (PINA & _BV(i)) {
                if (buttons[i]) {
                    buttons[i] = HELD;
                } else {
                    buttons[i] = DOWN;
                }
            } else if (buttons[i]) {
                buttons[i] = UP;
            }
        }

        /* Do Tetris logic. */
        if (buttons[0] == DOWN) {
            if (sprite0.y && !sprite_will_collide_left()) {
                sprite0.y--;
            }
        }
        if (buttons[1] == DOWN) {
            rotate_sprite_ccw();
        }
        if (buttons[3] == DOWN) {
            if (sprite0.y + sprite0.w < 8 && !sprite_will_collide_right()) {
                sprite0.y++;
            }
        }

        clear_full_lines();

        /* Mute, if switch 7 is set. */
        enable_audio(PINA & _BV(PA7));

        /* Check timer to advance the roll. */
        if (TIFR3 & _BV(OCF3A)) {
            TIFR3 = _BV(OCF3A);
            duration--;
            TCNT3 = 0;

            toggle++;

            if (toggle >= 5) {
                toggle = 0;
                change_state();
            }
        }

        if (!duration) {
            key_idx++;
            if (key_idx >= ROLL_SIZE) {
                key_idx = 0;
            }
            note = roll + key_idx;
            set_audio(note->pitch);

            duration = note->duration;
        }
    }
}
