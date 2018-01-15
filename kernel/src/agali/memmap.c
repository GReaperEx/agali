#include <agali/memmap.h>
#include <agali/textui.h>
#include <string.h>

#define MAX_MEMMAP_SIZE 64

typedef struct {
    uint64 base;
    uint64 size;
    uint32 type;
    uint32 extBitfield;
} memmapEntry;

static struct {
    uint64 amount;
    memmapEntry entries[MAX_MEMMAP_SIZE];
} memmap;

void mm_addToSorted(memmapEntry mmEntry);
int mm_findEntry(void* address);

void memmap_init(void)
{
    uint32 i;
#ifndef NDEBUG
    char buffer[32];
#endif // NDEBUG

    memmapEntry* mmBIOS = (memmapEntry*)0x508;
    uint32 mmBIOSsize = *((uint32*)0x500);

    if (mmBIOSsize > MAX_MEMMAP_SIZE) {
        textui_puts("Warning: Memory map was bigger than expected!\n");
    }

    DEBUG_PRINT("Detected memory map:\n");
    for (i = 0; i < mmBIOSsize; ++i) {
        if ((mmBIOS[i].type >= 1 && mmBIOS[i].type <= 5) && (mmBIOS[i].extBitfield & 0x3) == 1) {
            memmap.entries[memmap.amount++] = mmBIOS[i];
        }
#ifndef NDEBUG
        textui_puts("\tBase: ");
        textui_puts(int2str(mmBIOS[i].base, 16, buffer, sizeof(buffer)));
        textui_putchar(' ');
        textui_puts("Size: ");
        textui_puts(int2str(mmBIOS[i].size, 16, buffer, sizeof(buffer)));
        textui_putchar(' ');
        textui_puts("Type: ");
        textui_puts(int2str(mmBIOS[i].type, 10, buffer, sizeof(buffer)));
        textui_putchar(' ');
        textui_puts("ExType: ");
        textui_puts(int2str(mmBIOS[i].extBitfield, 2, buffer, sizeof(buffer)));
        textui_putchar('\n');
#endif // NDEBUG
    }

    for (i = 0; i < memmap.amount - 1; ++i) {
        uint64 base = memmap.entries[i].base;
        uint64 size = memmap.entries[i].size;
        uint64 otherBase = memmap.entries[i+1].base;

        if (base + size > otherBase) {
            uint64 overlap = base + size - otherBase;
            memmap.entries[i].size -= overlap;
            memmap.entries[i+1].base += overlap;
        }
    }
}

int mm_findEntry(void* address)
{
    uint64 i;
    uint64 addr = (uint64)address;

    for (i = 0; i < memmap.amount; ++i) {
        if (memmap.entries[i].base <= addr && memmap.entries[i].base + memmap.entries[i].size > addr) {
            break;
        }
    }

    return i;
}

BOOL memmap_isMemoryUsable(void* address, uint64 size)
{
    uint64 base = (uint64)address;
    uint64 maxSize;

    int index1 = mm_findEntry((void*)address);
    int index2 = mm_findEntry((void*)(base + size));

    if (index1 == memmap.amount || index2 == memmap.amount || index1 != index2) {
        return FALSE;
    }

    return memmap.entries[index1].type == 1 && memmap.entries[index1].base <= base &&
           memmap.entries[index1].base + memmap.entries[index1].size >= base + size;
}

void memmap_reclaimACPI(void)
{
    unsigned i;
    for (i = 0; i < memmap.amount; ++i) {
        if (memmap.entries[i].type == 3) {
            memmap.entries[i].type = 1;
        }
    }
}

void memmap_getMemorySizes(uint64* usable, uint64* reserved, uint64* acpi, uint64* nvs, uint64* bad)
{
    unsigned i;

    if (usable != NULL) {
        *usable = 0;
    }
    if (reserved != NULL) {
        *reserved = 0;
    }
    if (acpi != NULL) {
        *acpi = 0;
    }
    if (nvs != NULL) {
        *nvs = 0;
    }
    if (bad != NULL) {
        *bad = 0;
    }
    for (i = 0; i < memmap.amount; ++i) {
        switch (memmap.entries[i].type)
        {
        case 1:
            if (usable != NULL) {
                *usable += memmap.entries[i].size;
            }
        break;
        case 2:
            if (reserved != NULL) {
                *reserved += memmap.entries[i].size;
            }
        break;
        case 3:
            if (acpi != NULL) {
                *acpi += memmap.entries[i].size;
            }
        break;
        case 4:
            if (nvs != NULL) {
                *nvs += memmap.entries[i].size;
            }
        break;
        case 5:
            if (bad != NULL) {
                *bad += memmap.entries[i].size;
            }
        break;
        }
    }
}

uint64 memmap_getMemUpperBound(void)
{
    return memmap.entries[memmap.amount - 1].base + memmap.entries[memmap.amount - 1].size;
}
