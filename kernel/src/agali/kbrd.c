#include <agali/kbrd.h>
#include <agali/ktypes.h>
#include <agali/descriptors.h>
#include <agali/pic.h>
#include <agali/portio.h>
#include <ctype.h>
#include <string.h>

extern void ISR33(void);

static const char asciiMap[2][2][_KEY_AMOUNT] = {
       // No NumLock, No Shift
    { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\\', '\b', 0, 0, 0, 0, 0, 0, 0,
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 0, 0, 0, 0, 0, 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0, 0, 0,
        0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 0, 0, 0,
        0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0 },
       // No Numlock, Shift
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '|', '\b', 0, 0, 0, 0, 0, 0, 0,
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', 0, 0, 0, 0, 0, 0, 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', 0, 0, 0,
        0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, 0, 0, 0, 0, 0,
        0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
       // Numlock, No Shift
    { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\\', '\b', 0, 0, 0, 0, '/', '*', '-',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 0, 0, '7', '8', '9', '+',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '4', '5', '6',
        0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, '1', '2', '3', '\n',
        0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, '0', '.' },
       // Numlock, Shift
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '|', '\b', 0, 0, 0, 0, '/', '*', '-',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', 0, 0, 0, '7', '8', '9', '+',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '4', '5', '6',
        0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, 0, '1', '2', '3', '\n',
        0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, '0', '.' } }
};

volatile struct {
    char lastChar;
    kbrd_keys lastKey;

    BOOL scrollLock;
    BOOL numLock;
    BOOL capsLock;
    uint8 keyState[_KEY_AMOUNT];
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

    idt_enable(33, (intptr)ISR33, 0x08, GATE_INT, TRUE);
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

BOOL kbrd_isLEDon(kbrd_LEDs led)
{
    switch (led)
    {
    case KBRD_SCROLLOCK_LED:
        return kbrd_state.scrollLock;
    break;
    case KBRD_NUMLOCK_LED:
        return kbrd_state.numLock;
    break;
    case KBRD_CAPSLOCK_LED:
        return kbrd_state.capsLock;
    break;
    }
    return FALSE;
}

BOOL kbrd_isKeyPressed(kbrd_keys key)
{
    if (key >= 0 && key < _KEY_AMOUNT) {
        return kbrd_state.keyState[key];
    }
    return FALSE;
}

static kbrd_keys translateScancode(uint8* buffer, int bufSize, BOOL* wasPressed);

void kbrd_handler(void)
{
    static uint8 buffer[4] = { 0 };
    static int bufCurSize = 0;
    kbrd_keys curKey = _KEY_AMOUNT;
    BOOL pressed;
    int c = inb(0x60);

    if (c != 0xFA && c != 0xFE) {
        buffer[bufCurSize++] = inb(0x60);
        curKey = translateScancode(buffer, bufCurSize, &pressed);
    }

    if (curKey < _KEY_AMOUNT) {
        if (pressed) {
            kbrd_state.lastKey = curKey;
            kbrd_state.keyState[curKey] = 1;

            kbrd_state.lastChar = asciiMap[kbrd_state.numLock][(kbrd_state.keyState[KEY_LSHIFT] || kbrd_state.keyState[KEY_RSHIFT])][curKey];

            if (kbrd_state.capsLock ^ (kbrd_state.keyState[KEY_LSHIFT] || kbrd_state.keyState[KEY_RSHIFT])) {
                kbrd_state.lastChar = toupper(kbrd_state.lastChar);
            }

            if (curKey == KEY_SCROLLOCK) {
                kbrd_state.scrollLock = !kbrd_state.scrollLock;
                _setLEDs();
            } else if (curKey == KEY_NUMLOCK) {
                kbrd_state.numLock = !kbrd_state.numLock;
                _setLEDs();
            } else if (curKey == KEY_CAPSLOCK) {
                kbrd_state.capsLock = !kbrd_state.capsLock;
                _setLEDs();
            }
        } else {
            kbrd_state.keyState[curKey] = 0;
        }
        bufCurSize = 0;
    }
    if (bufCurSize > sizeof(buffer)) {
        bufCurSize = 0;
    }

    pic_sendEOI(1, 0);
}

static uint8 scanCodes[2][128] = {
    { _KEY_AMOUNT, KEY_ESCAPE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6,
      KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS, KEY_BSPACE, KEY_TAB,
      KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I,
      KEY_O, KEY_P, KEY_LBRACKET, KEY_RBRACKET, KEY_ENTER, KEY_LCTRL, KEY_A, KEY_S,
      KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLOR,
      KEY_SQUOTE, KEY_BTICK, KEY_LSHIFT, KEY_BSLASH, KEY_Z, KEY_X, KEY_C, KEY_V,
      KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RSHIFT, KEY_PAD_ASTERISK,
      KEY_LALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
      KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK, KEY_SCROLLOCK, KEY_PAD_7,
      KEY_PAD_8, KEY_PAD_9, KEY_PAD_MINUS, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6, KEY_PAD_PLUS, KEY_PAD_1,
      KEY_PAD_2, KEY_PAD_3, KEY_PAD_0, KEY_PAD_PERIOD, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, KEY_F11,
      KEY_F12, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT },

    { _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      KEY_PREV_TRACK, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, KEY_NEXT_TRACK, _KEY_AMOUNT, _KEY_AMOUNT, KEY_PAD_ENTER, KEY_RCTRL, _KEY_AMOUNT, _KEY_AMOUNT,
      KEY_MUTE, KEY_CALC, KEY_PLAY_PAUSE, _KEY_AMOUNT, KEY_STOP, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, KEY_VOL_DOWN, _KEY_AMOUNT,
      KEY_VOL_UP, _KEY_AMOUNT, KEY_WWW_HOME, _KEY_AMOUNT, _KEY_AMOUNT, KEY_PAD_SLASH, _KEY_AMOUNT, _KEY_AMOUNT,
      KEY_RALT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, KEY_HOME,
      KEY_UP, KEY_PGUP, _KEY_AMOUNT, KEY_LEFT, _KEY_AMOUNT, KEY_RIGHT, _KEY_AMOUNT, KEY_END,
      KEY_DOWN, KEY_PGDOWN, KEY_INSERT, KEY_DELETE, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, KEY_LGUI, KEY_RGUI, KEY_APP, KEY_POWER, KEY_SLEEP,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, KEY_WAKE, _KEY_AMOUNT, KEY_WWW_SEARCH, KEY_WWW_FAV, KEY_WWW_REFRESH,
      KEY_WWW_STOP, KEY_WWW_FORW, KEY_WWW_BACK, KEY_MYCOM, KEY_EMAIL, KEY_MEDIA_SEL, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT,
      _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT, _KEY_AMOUNT }
};

static kbrd_keys translateScancode(uint8* buffer, int bufSize, BOOL* wasPressed)
{
    switch (bufSize)
    {
    case 1:
        if (buffer[0] != 0xE0 && buffer[0] != 0xE1) {
            *wasPressed = (buffer[0] & 0x80) == 0;
            return scanCodes[0][buffer[0] & 0x7F];
        }
    break;
    case 2:
        if (buffer[0] == 0xE0 && buffer[1] != 0x2A && buffer[1] != 0xB7) {
            *wasPressed = (buffer[1] & 0x80) == 0;
            return scanCodes[0][buffer[1] & 0x7F];
        }
    break;
    case 3:
        if (buffer[0] == 0xE1 && (buffer[1] & 0x7F) == 0x1D && (buffer[2] & 0x7D) == 0x45 && (buffer[1] & 0x80) == (buffer[2] & 0x80)) {
            *wasPressed = (buffer[1] & 0x80) == 0;
            return KEY_PAUSE;
        }
    break;
    case 4:
        if (buffer[0] == 0xE0 && buffer[1] == 0x2A && buffer[2] == 0xE0 && buffer[3] == 0x37) {
            *wasPressed = TRUE;
            return KEY_PRTSCR;
        } else if (buffer[0] == 0xE0 && buffer[1] == 0xB7 && buffer[2] == 0xE0 && buffer[3] == 0xAA) {
            *wasPressed = FALSE;
            return KEY_PRTSCR;
        }
    break;
    }
    return _KEY_AMOUNT;
}
