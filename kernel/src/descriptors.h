#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "ktypes.h"

typedef struct __attribute__((packed)) {
    uint16 limit_low;
    uint16 base_low;

    struct {
        uint8 Ac    : 1;
        uint8 RW    : 1; // Is segment Read/Write
        uint8 DC    : 1; // Direction/Conforming bit
        uint8 Ex    : 1;
        uint8 _res  : 1; // 1
        uint8 Privl : 2;
        uint8 P     : 1;
    } access;

    struct {
        uint8 limit_high : 4;
        uint8 _res       : 1; // 0
        uint8 L          : 1; // Specifies long mode segment
        uint8 Sz         : 1;
        uint8 Gr         : 1; // 1
    };

    uint8 base_high;
} gdt_descr;

typedef struct __attribute__((packed)) {
    uint16 limit;
    uint64 offset;
} GDTR;

typedef struct __attribute__((packed)) {
    uint16 offset_1;
    uint16 selector;
    struct {
        uint8 IST : 3; // Interrupt Stack Table offset
    };
    struct {
        uint8 type : 4; // Gate Type
        uint8 _res : 1; // 0
        uint8 DPL  : 2;
        uint8 P    : 1;
    };
    uint16 offset_2;
    uint32 offset_3;
    uint32 zero;
} idt_descr;

typedef struct __attribute__((packed)) {
    uint16 limit;
    uint64 offset;
} IDTR;

typedef enum { GATE_TASK = 0x5, GATE_INT = 0xE, GATE_TRAP = 0xF } GateType;
typedef void (*ISRptr)(void);

static inline gdt_descr makeGDTdescr(BOOL isCode, BOOL isSuper)
{
    gdt_descr newDescr = {
        0x0,
        0x0,
        { 0, 1, 0, isCode, 1, (isSuper ? 0 : 3), 1 },
        { 0x0, 0, 1, 0, 1 },
        0x0
    };
    return newDescr;
}

static inline idt_descr makeIDTdescr(intptr handler, uint16 selector, GateType type, BOOL isSuper)
{
    idt_descr newDescr = {
        handler,
        selector,
        { 0x0 },
        { type, 0, (isSuper ? 0 : 3), 1 },
        handler >> 16, handler >> 32, 0
    };
    return newDescr;
}

static inline void loadGDTreg(GDTR* GDTptr)
{
    __asm__ __volatile__("lgdt (%0)" :: "r"(GDTptr) : "memory");
}

static inline void loadIDTreg(IDTR* IDTptr)
{
    __asm__ __volatile__("lidt (%0)" :: "r"(IDTptr) : "memory");
}

void gdt_init(void);
void idt_init(void);
void idt_enable(int intIndex, intptr handler, uint16 selector, GateType type, BOOL isSuper);
void idt_disable(int intIndex);

#endif // DESCRIPTORS_H
