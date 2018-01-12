#ifndef GDT_H
#define GDT_H

#include "ktypes.h"

typedef union {
    struct {
        uint64 limit_low  : 16;
        uint64 base_low   : 24;
        uint64 access     : 8;
        uint64 limit_high : 4;
        uint64 flags      : 4;
        uint64 base_high  : 8;
    };
    uint64 rvalue;
} gdt_descr;

void gdt_init(void);

#endif // GDT_H
