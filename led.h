#ifndef LED_H
#define LED_H

extern unsigned char green_plane[8];
extern unsigned char red_plane[8];

struct sprite {
    unsigned char red[8], green[8], x, y;
};

extern struct sprite sprite0;

void start_scanout();

#endif
