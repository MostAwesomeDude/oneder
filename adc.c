/**
  @file adc.c
  @brief Wunderboard ADC Helper Functions
  @version .01

  @section intro Code Overview
  This is the code for the Wunderboard ADC helper functions.
  */

#include "adc.h"

unsigned char read_adc(uint8_t channel) {

    unsigned char test;

    /* Enable ADC and set the channel to the desirec channel. */
    ADMUX = _BV(REFS0) | _BV(ADLAR) | channel;

    /* Start a new sample. */
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADPS2) | _BV(ADPS1);

    /* Get a valid sample. */
    loop_until_bit_is_set(ADCSRA, ADIF);

    /* Acknowledge the sample. */
    ADCSRA |= _BV(ADIF);

    /* Get another one. */
    ADCSRA |= _BV(ADSC);
    loop_until_bit_is_set(ADCSRA, ADIF);
    ADCSRA |= _BV(ADIF);

    /* Get our sample. */
    test = ADCH;

    /* And turn ADC back off. */
    ADCSRA = 0x00;

    return test;
}
