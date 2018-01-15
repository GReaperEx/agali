#ifndef STDIO_H
#define STDIO_H

#include <agali/textui.h>
#include <stddef.h>

#define EOF -1
#define putchar(c) textui_putchar(c)
#define puts(s) textui_puts(s)

typedef long fpos_t;

int printf(const char* format, ...);

#endif // STDIO_H
