#include "gdt.h"

static gdt_descr GDT[5];
static struct __attribute__((packed)) {
    uint16 size;
    uint64 offset;
} GDTptr;

void resetSegments(void);

void gdt_init(void)
{
    GDT[1].access = 0x9A; // Kernel code segment
    GDT[1].flags = 0x2;
    GDT[2].access = 0x92; // Kernel data segment
    GDT[2].flags = 0x2;
    GDT[3].access = 0xFA; // User code segment
    GDT[3].flags = 0x2;
    GDT[4].access = 0xF2; // User data segment
    GDT[4].flags = 0x2;

    GDTptr.size = sizeof(GDT) - 1;
    GDTptr.offset = (uint64)GDT;

    __asm__ __volatile__(
        "lgdt (%0) \n\t"
     :: "r"(&GDTptr) : "memory"
    );
}
