#include "textui.h"
#include "memmap.h"
#include "kstring.h"
#include "sse.h"
#include "paging.h"
#include "exceptions.h"

#include "descriptors.h"
#include "acpi.h"

void kmain(void)
{
#ifndef NDEBUG
    char buffer[32];
    uint64 memSizes[5];
#endif // NDEBUG

	textui_init();
	textui_setColor(0x7, 0x0);
	textui_clrscr();

	DEBUG_PRINT("Hello World, agali locked and loaded!\n\n");

	DEBUG_PRINT("Detecting memory.\n");
    memmap_init();
#ifndef NDEBUG
    memmap_getMemorySizes(&memSizes[0], &memSizes[1], &memSizes[2], &memSizes[3], &memSizes[4]);
    textui_puts("Usable Reserved ACPI Non-volatile Bad\n");
    textui_puts(int2str(memSizes[0], 16, buffer, sizeof(buffer)));
    textui_putchar(' ');
    textui_puts(int2str(memSizes[1], 16, buffer, sizeof(buffer)));
    textui_putchar(' ');
    textui_puts(int2str(memSizes[2], 16, buffer, sizeof(buffer)));
    textui_putchar(' ');
    textui_puts(int2str(memSizes[3], 16, buffer, sizeof(buffer)));
    textui_putchar(' ');
    textui_puts(int2str(memSizes[4], 16, buffer, sizeof(buffer)));
    textui_putchar('\n');
#endif // NDEBUG

    DEBUG_PRINT("Enabling SSE.\n");
    sse_enable();

    DEBUG_PRINT("Reinitializing GDT.\n");
    gdt_init();

    idt_init();
    DEBUG_PRINT("Enabling CPU exceptions.\n");
    exceptions_init();

    DEBUG_PRINT("Reinitializing paging.\n");
    paging_init();

    if (!acpi_init()) {
        textui_puts("This system doesn't appear to support ACPI!\n");
    } else {
        DEBUG_PRINT("Detected ACPI Root System Description Pointer.\n");
        if (acpi_findHeader("APIC")) {
            textui_puts("Found the ACPI APIC table.\n");
        }
    }

	for (;;) {
	    __asm__ __volatile__("hlt \n\t");
	}
}
