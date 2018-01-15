#ifndef TEXTUI_H
#define TEXTUI_H

#include <agali/ktypes.h>

void textui_init(void);

void textui_getColor(int* fg, int* bg);
void textui_setColor(int fg, int bg);

BOOL textui_isCursorVisible(void);
void textui_showCursor(BOOL show);

void textui_getCursorPos(int* curCol, int* curRow);
void textui_setCursorPos(int curCol, int curRow);

void textui_clrscr(void);

void textui_putchar(int c);
void textui_puts(const char* str);

#endif // TEXTUI_H
