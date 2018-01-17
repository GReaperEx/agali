#ifndef STDIO_H
#define STDIO_H

#include <agali/textui.h>
#include <agali/kbrd.h>
#include <stddef.h>

#define EOF -1

typedef long fpos_t;

static inline int putchar(int c)
{
    textui_putchar(c);
    return 0;
}

int puts(const char* str);
int printf(const char* format, ...);

int getchar(void);
char* gets(char* dst, int n);

#endif // STDIO_H
