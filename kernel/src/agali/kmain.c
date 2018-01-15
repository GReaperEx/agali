#include <agali/textui.h>
#include <agali/memmap.h>
#include <agali/sse.h>
#include <agali/paging.h>
#include <agali/exceptions.h>
#include <agali/descriptors.h>
#include <agali/acpi.h>

#include <stdio.h>
#include <string.h>

void kmain(void)
{
#ifndef NDEBUG
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
    printf("%-10s %-10s %-10s %-10s %-10s\n", "Usable", "Reserved", "ACPI", "Volatile", "Bad");
    printf("%-10lX %-10lX %-10lX %-10lX %-10lX\n", memSizes[0], memSizes[1], memSizes[2], memSizes[3], memSizes[4]);
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
        if (acpi_findTable("APIC")) {
            textui_puts("Found the ACPI APIC table.\n");
        }
    }

	for (;;) {
	    __asm__ __volatile__("hlt \n\t");
	}
}
