#ifndef STDIO_H
#define STDIO_H

#include <agali/textui.h>
#include <stddef.h>

#define EOF -1
#define putchar(c) textui_putchar(c)

typedef long fpos_t;

int puts(const char* str);
int printf(const char* format, ...);

#endif // STDIO_H
