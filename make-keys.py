#!/usr/bin/env python

from __future__ import division

# Spit out a C array holding each of the lengths of the pitches of each note
# on an 88-key keyboard, by duration of wavelength in useconds.

def frequency_for_pitch(pitch):
    return 440 * 2**((pitch - 69)/12)

def us_for_frequency(freq):
    return 1000 * 1000 / freq

print """
/* keys.h */

#ifndef KEYS_H
#define KEYS_H

static unsigned short key_us[] = {
"""

print ",\n".join(" %d" % us_for_frequency(frequency_for_pitch(key))
    for key in range(128))

print """
};

#endif /* KEYS_H */
"""
