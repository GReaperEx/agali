#ifndef KTYPES_H
#define KTYPES_H

#include <ctype.h>
#include <stddef.h>

#ifndef NDEBUG
#define DEBUG_PRINT(s) textui_puts(s)
#else
#define DEBUG_PRINT(s) (void)
#endif // NDEBUG

typedef unsigned char uint8;
typedef signed char int8;

typedef unsigned short uint16;
typedef signed short int16;

typedef unsigned int uint32;
typedef signed int int32;

typedef unsigned long uint64;
typedef signed long int64;

typedef uint64 intptr;

typedef uint8 BOOL;
#define FALSE 0
#define TRUE 1

#endif // KTYPES_H
