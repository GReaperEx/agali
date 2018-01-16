#ifndef KBRD_H
#define KBRD_H

#include <agali/ktypes.h>

void kbrd_init(void);
int kbrd_getchar(void);

typedef enum {
    KBRD_SCROLLOCK_LED, KBRD_NUMLOCK_LED, KBRD_CAPSLOCK_LED
} kbrd_LEDs;

BOOL kbrd_isLEDon(kbrd_LEDs led);

#endif // KBRD_H
