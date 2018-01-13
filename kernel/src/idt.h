#ifndef IDT_H
#define IDT_H

#include "ktypes.h"

typedef struct __attribute__((packed)) {
   uint16 offset_1;
   uint16 selector;
   uint8 ist;
   uint8 type_attr;
   uint16 offset_2;
   uint32 offset_3;
   uint32 zero;
} idt_descr;

typedef enum { GATE_TASK = 0x5, GATE_INT = 0xE, GATE_TRAP = 0xF } GateType;
typedef void (*ISRptr)(void);

void idt_init(void);
void idt_enable(int intIndex, ISRptr isr, GateType type, BOOL isSuper);
void idt_disable(int intIndex);

#endif // IDT_H
