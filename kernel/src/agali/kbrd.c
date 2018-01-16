#include <agali/kbrd.h>
#include <agali/ktypes.h>
#include <agali/descriptors.h>
#include <agali/pic.h>
#include <agali/portio.h>
#include <ctype.h>
#include <string.h>

extern void ISR32(void);

typedef enum {
    KEY_ESCAPE = 1, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y,
    KEY_U, KEY_I, KEY_O, KEY_P, KEY_LBRACKET, KEY_RBRACKET, KEY_ENTER, KEY_LCONTROL, KEY_A,
    KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLOR, KEY_SQUOTE,
    KEY_BACKTICK, KEY_LSHIFT, KEY_BSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
    KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RSHIFT, KEY_PAD_MULT, KEY_LALT, KEY_SPACE, KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK,
    KEY_SCROLLOCK, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9, KEY_PAD_MINUS, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6,
    KEY_PAD_PLUS, KEY_PAD_1, KEY_PAD_2, KEY_PAD_3, KEY_PAD_0, KEY_PAD_PERIOD,
    KEY_F11 = 0x57, KEY_F12
} kbrd_keys;

static const char keyMap[2][128] = {
    { 0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
      0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
      'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+' },
    { 0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',
      0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 0, '\|',
      'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, '*', 0, ' ', 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+' }
};

volatile struct {
    char lastChar;
    kbrd_keys lastKey;

    BOOL scrollLock;
    BOOL numLock;
    BOOL capsLock;
    uint8 keyState[128];
} kbrd_state;

static void _setLEDs(void)
{
    outb(0x60, 0xED);
    io_wait();
    outb(0x60, kbrd_state.scrollLock | (kbrd_state.numLock << 1) | (kbrd_state.capsLock << 2));
}

void kbrd_init(void)
{
    _setLEDs();
    idt_enable(33, (intptr)ISR32, 0x08, GATE_INT, TRUE);
    pic_enableIRQ(1);
}

int kbrd_getchar(void)
{
    int c;

    while (kbrd_state.lastChar == 0) {
        __asm__ __volatile__("hlt \n\t");
    }

    c = kbrd_state.lastChar;
    kbrd_state.lastChar = 0;
    return c;
}

void kbrd_handler(void)
{
    static BOOL extended = FALSE;
    int c = inb(0x60);

    if (c != 0xE0) {
        if (!(c & 0x80) && c) {
            kbrd_state.lastKey = c;
            kbrd_state.keyState[c] = 1;

            if (kbrd_state.keyState[KEY_LSHIFT] || kbrd_state.keyState[KEY_RSHIFT]) {
                kbrd_state.lastChar = keyMap[1][c];
            } else {
                kbrd_state.lastChar = keyMap[0][c];
            }
            if (kbrd_state.capsLock ^ (kbrd_state.keyState[KEY_LSHIFT] || kbrd_state.keyState[KEY_RSHIFT])) {
                kbrd_state.lastChar = toupper(kbrd_state.lastChar);
            }

            switch (c)
            {
            case KEY_SCROLLOCK:
                kbrd_state.scrollLock = !kbrd_state.scrollLock;
                _setLEDs();
            break;
            case KEY_NUMLOCK:
                kbrd_state.numLock = !kbrd_state.numLock;
                _setLEDs();
            break;
            case KEY_CAPSLOCK:
                kbrd_state.capsLock = !kbrd_state.capsLock;
                _setLEDs();
            break;
            }
        } else {
            c &= 0x7F;
            kbrd_state.keyState[c] = 0;
        }
        extended = FALSE;
    } else {
        extended = TRUE;
    }

    pic_sendEOI(1, 0);
}
