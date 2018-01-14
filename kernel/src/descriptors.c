#include "descriptors.h"

static idt_descr IDT[256];
static gdt_descr GDT[5];
static GDTR GDTptr;
static IDTR IDTptr;

void gdt_init(void)
{
    GDT[1] = makeGDTdescr(TRUE, TRUE);
    GDT[2] = makeGDTdescr(FALSE, TRUE);
    GDT[3] = makeGDTdescr(TRUE, FALSE);
    GDT[4] = makeGDTdescr(FALSE, FALSE);

    GDTptr.limit = sizeof(GDT) - 1;
    GDTptr.offset = (uint64)GDT;

    loadGDTreg(&GDTptr);
}

void idt_init(void)
{
    IDTptr.limit = sizeof(IDT) - 1;
    IDTptr.offset = (uint64)IDT;

    loadIDTreg(&IDTptr);
}

void idt_enable(int intIndex, intptr handler, uint16 selector, GateType type, BOOL isSuper)
{
    if (intIndex >= 0 && intIndex < 256) {
        IDT[intIndex] = makeIDTdescr(handler, selector, type, isSuper);
    }
}

void idt_disable(int intIndex)
{
    if (intIndex >= 0 && intIndex < 256) {
        IDT[intIndex].P = 0;
    }
}
