#include <agali/textui.h>
#include <agali/portio.h>

static struct {
    uint16* videoBase;

    int curColumn;
    int curRow;
    int maxColumns;
    int maxRows;

    uint8 curFgColor;
    uint8 curBgColor;
    BOOL cursorVisible;
} textui_info;

void textui_init(void)
{
    uint16* BDA = (uint16*)0x400;

    if ((BDA[8] & 0x30) == 0x30) {
        textui_info.videoBase = (uint16*)0xB0000;
    } else {
        textui_info.videoBase = (uint16*)0xB8000;
    }

    textui_info.curColumn = textui_info.curRow = 0;
    textui_info.maxColumns = BDA[37];
    textui_info.maxRows = 25;

    textui_info.curFgColor = 0xF;
    textui_info.curBgColor = 0x0;
    textui_info.cursorVisible = TRUE;

    // Enabling the cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0));
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D0) & 0xE0) | 0xC);

    textui_showCursor(FALSE);
}

void textui_getColor(int* fg, int* bg)
{
    if (fg != NULL) {
        *fg = textui_info.curFgColor;
    }
    if (bg != NULL) {
        *bg = textui_info.curBgColor;
    }
}

void textui_setColor(int fg, int bg)
{
    if (fg >= 0) {
        textui_info.curFgColor = fg & 0xF;
    }
    if (bg >= 0) {
        textui_info.curBgColor = bg & 0xF;
    }
}

BOOL textui_isCursorVisible(void)
{
    return textui_info.cursorVisible;
}

void textui_showCursor(BOOL show)
{
    if (show) {
        uint16 pos = textui_info.curRow*textui_info.maxColumns + textui_info.curColumn;

        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8)(pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8)((pos >> 8) & 0xFF));
    } else {
        uint16 pos = textui_info.maxRows*textui_info.maxColumns + textui_info.maxColumns;

        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8)(pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8)((pos >> 8) & 0xFF));
    }
    textui_info.cursorVisible = show;
}

void textui_getCursorPos(int* curCol, int* curRow)
{
    if (curCol != NULL) {
        *curCol = textui_info.curColumn;
    }
    if (curRow != NULL) {
        *curRow = textui_info.curRow;
    }
}

void textui_setCursorPos(int curCol, int curRow)
{
    if (curCol >= 0) {
        textui_info.curColumn = curCol;
    }
    if (curRow >= 0) {
        textui_info.curRow = curRow;
    }
}

void textui_clrscr(void)
{
    int i, areaSize = textui_info.maxColumns*textui_info.maxRows;
    uint16 fillWith = (textui_info.curBgColor << 12) |
                      (textui_info.curFgColor <<  8) | ' ';

    for (i = 0; i < areaSize; ++i) {
        textui_info.videoBase[i] = fillWith;
    }
}

static void _scrollLine(void);

void textui_putchar(int c)
{
    uint16 fillWith = (textui_info.curBgColor << 12) |
                      (textui_info.curFgColor <<  8) | (c & 0xFF);
    int index = textui_info.curRow*textui_info.maxColumns + textui_info.curColumn;

    c &= 0xFF;
    switch (c)
    {
    case '\r':
    case '\v':
    case '\a':
    break;
    case '\n':
        textui_info.curColumn = 0;
        if (++textui_info.curRow >= textui_info.maxRows) {
            textui_info.curRow = textui_info.maxRows - 1;
            _scrollLine();
        }
    break;
    case '\t':
        textui_info.curColumn = (textui_info.curColumn + 4) & 0xFFFFFFFC;
    break;
    case '\b':
        if (--textui_info.curColumn < 0) {
            textui_info.curColumn = 0;
        }
        textui_putchar(' ');
        --textui_info.curColumn;
    break;
    default:
        textui_info.videoBase[index] = fillWith;
        if (++textui_info.curColumn >= textui_info.maxColumns) {
            textui_info.curColumn = 0;
            if (++textui_info.curRow >= textui_info.maxRows) {
                textui_info.curRow = textui_info.maxRows - 1;
                _scrollLine();
            }
        }
    }
}

void textui_puts(const char* str)
{
    int c;
    while ((c = *str++) != '\0') {
        textui_putchar(c);
    }
}

static void _scrollLine(void)
{
    int i;
    int areaSize = textui_info.maxColumns*textui_info.maxRows;

    for (i = textui_info.maxColumns; i < areaSize; ++i) {
        textui_info.videoBase[i - textui_info.maxColumns] = textui_info.videoBase[i];
    }
    for (i = i - textui_info.maxColumns; i < areaSize; ++i) {
        textui_info.videoBase[i] = (textui_info.videoBase[i] & 0xFF00) | ' ';
    }
}
