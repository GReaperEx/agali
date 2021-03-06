#ifndef KBRD_H
#define KBRD_H

#include <agali/ktypes.h>

void kbrd_init(void);
int kbrd_getchar(void);

typedef enum {
    KBRD_SCROLLOCK_LED, KBRD_NUMLOCK_LED, KBRD_CAPSLOCK_LED
} kbrd_LEDs;

BOOL kbrd_isLEDon(kbrd_LEDs led);

typedef enum {
// Standard US QWERTY layout
    KEY_ESCAPE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_PRTSCR, KEY_SCROLLOCK, KEY_PAUSE,
    KEY_BTICK, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS, KEY_BSLASH,
    KEY_BSPACE, KEY_INSERT, KEY_HOME, KEY_PGUP, KEY_NUMLOCK, KEY_PAD_SLASH, KEY_PAD_ASTERISK, KEY_PAD_MINUS,
    KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LBRACKET, KEY_RBRACKET, KEY_ENTER,
    KEY_DELETE, KEY_END, KEY_PGDOWN, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9, KEY_PAD_PLUS,
    KEY_CAPSLOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLOR, KEY_SQUOTE, KEY_PAD_4,
    KEY_PAD_5, KEY_PAD_6,
    KEY_LSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RSHIFT, KEY_UP,
    KEY_PAD_1, KEY_PAD_2, KEY_PAD_3, KEY_PAD_ENTER,
    KEY_LCTRL, KEY_LGUI, KEY_LALT, KEY_SPACE, KEY_RALT, KEY_RGUI, KEY_APP, KEY_RCTRL, KEY_LEFT, KEY_DOWN, KEY_RIGHT,
    KEY_PAD_0, KEY_PAD_PERIOD,
// Extra keys
    KEY_POWER, KEY_SLEEP, KEY_WAKE,
    KEY_NEXT_TRACK, KEY_PREV_TRACK, KEY_STOP, KEY_PLAY_PAUSE, KEY_MUTE, KEY_VOL_UP, KEY_VOL_DOWN, KEY_MEDIA_SEL, KEY_EMAIL,
    KEY_CALC, KEY_MYCOM, KEY_WWW_SEARCH, KEY_WWW_HOME, KEY_WWW_BACK, KEY_WWW_FORW, KEY_WWW_STOP, KEY_WWW_REFRESH, KEY_WWW_FAV,
    _KEY_AMOUNT
} kbrd_keys;

BOOL kbrd_isKeyPressed(kbrd_keys key);

#endif // KBRD_H
