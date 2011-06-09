#include <avr/io.h>

#include "keys.h"

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

/* Start the audio timer.
 * It is probably a good idea to call set_audio() sometime soon after doing
 * this. */
void start_audio() {
    /* Enable audio out. */
    PORTB |= _BV(PB5);

    /* Start timer for audio: CTC, toggle on match, no prescaling. */
    TCCR1A = _BV(COM1A0);
    TCCR1B = _BV(WGM12) | _BV(CS10);
}

void enable_audio(unsigned char enable) {
    if (enable) {
        TCCR1A &= ~_BV(COM1A0);
    } else {
        TCCR1A = _BV(COM1A0);
    }
}
