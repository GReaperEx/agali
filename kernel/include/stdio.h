#ifndef STDIO_H
#define STDIO_H

#include <agali/textui.h>
#include <agali/kbrd.h>
#include <stddef.h>

#define EOF -1
#define putchar(c) textui_putchar(c)

typedef long fpos_t;

int puts(const char* str);
int printf(const char* format, ...);

int getchar(void);
char* gets(char* dst, int n);

#endif // STDIO_H
