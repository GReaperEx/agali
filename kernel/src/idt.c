#include "idt.h"
#include "textui.h"

static idt_descr IDT[256];
static struct __attribute__((packed)) {
    uint16 limit;
    uint64 base;
} IDTR;

void idt_init(void)
{
    IDTR.limit = sizeof(IDT) - 1;
    IDTR.base = (uint64)IDT;

    __asm__ __volatile__(
        "lidt (%0) \n\t"
    :: "r"(&IDTR)
    );
}

void idt_enable(int intIndex, ISRptr isr, GateType type, BOOL isSuper)
{
    if (intIndex >= 0 && intIndex < 256) {
        IDT[intIndex].type_attr &= 0x7F;

        IDT[intIndex].offset_1 = ((uint64)isr >> 0) & 0xFFFF;
        IDT[intIndex].offset_2 = ((uint64)isr >> 16) & 0xFFFF;
        IDT[intIndex].offset_3 = ((uint64)isr >> 32) & 0xFFFFFFFF;
        IDT[intIndex].selector = 0x08;
        IDT[intIndex].ist = 0;
        IDT[intIndex].zero = 0;
        IDT[intIndex].type_attr = 0x80 | type | ((isSuper ? 0 : 3) << 5);
    } else {
        textui_puts("Warning: Attempted to enable a non-existent interrupt!");
    }
}

void idt_disable(int intIndex)
{
    if (intIndex >= 0 && intIndex < 256) {
        IDT[intIndex].type_attr &= 0x7F;
    } else {
        textui_puts("Warning: Attempted to disable a non-existent interrupt!");
    }
}
