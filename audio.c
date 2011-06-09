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
