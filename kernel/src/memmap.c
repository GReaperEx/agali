#include "memmap.h"
#include "textui.h"
#include "kstring.h"

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
            mm_addToSorted(mmBIOS[i]);
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

    DEBUG_PRINT("Cleaned and sorted memory map:\n");
#ifndef NDEBUG
    for (i = 0; i < memmap.amount; ++i) {
        textui_puts("\tBase: ");
        textui_puts(int2str(memmap.entries[i].base, 16, buffer, sizeof(buffer)));
        textui_putchar(' ');
        textui_puts("Size: ");
        textui_puts(int2str(memmap.entries[i].size, 16, buffer, sizeof(buffer)));
        textui_putchar(' ');
        textui_puts("Type: ");
        textui_puts(int2str(memmap.entries[i].type, 10, buffer, sizeof(buffer)));
        textui_putchar('\n');
    }
#endif // NDEBUG
}

void mm_addToSorted(memmapEntry mmEntry)
{
    int i, index;

    index = mm_findEntry((void*)mmEntry.base);
    ++memmap.amount;

    if (memmap.amount > 0 && memmap.entries[index].base <= mmEntry.base) {
        ++index;
    }

    for (i = memmap.amount - 1; i > index; --i) {
        memmap.entries[i] = memmap.entries[i-1];
    }
    memmap.entries[i] = mmEntry;
}

int mm_findEntry(void* address)
{
    uint64 l = 0, r = memmap.amount - 1;
    uint64 addr = (uint64)address;

    if (memmap.amount == 0) {
        return 0;
    }

    while (l < r)
    {
        uint64 m = l + (r-l)/2;
        uint64 base = memmap.entries[m].base;

        if (base < addr) {
            l = m + 1;
        } else if (base + memmap.entries[m].size >= addr){
            r = m - 1;
        } else {
            break;
        }
    }

    return l + (r-l)/2;
}

BOOL memmap_isMemoryUsable(void* address, uint64 size)
{
    uint64 base = (uint64)address;
    uint64 maxSize;

    int index1 = mm_findEntry((void*)address);
    int index2 = mm_findEntry((void*)(base + size));

    if (index1 == index2) {
        return memmap.entries[index1].type == 1 && memmap.entries[index1].base >= base &&
               memmap.entries[index1].base + memmap.entries[index1].size >= base + size;
    }

    if (memmap.entries[index1].type == 1 && memmap.entries[index1].base >= base &&
        memmap.entries[index1].base + memmap.entries[index1].size > base &&
        memmap.entries[index2].type == 1 && memmap.entries[index2].base < base + size &&
        memmap.entries[index2].base + memmap.entries[index2].size >= base + size) {
        // Sum everything in-between to see if it fits
        maxSize = (memmap.entries[index1].base + memmap.entries[index1].size) - base;
        if (maxSize < size) {
            for (; index1 <= index2; ++index1) {
                if (memmap.entries[index1].type != 1) {
                    break;
                }
                size = memmap.entries[index1].size;
                if (maxSize >= size) {
                    size = 0;
                    break;
                }
                size -= maxSize;
            }
        } else {
            size = 0;
        }
    }

    return size == 0;
}

void memmap_reclaimACPI(void)
{
    int i;
    for (i = 0; i < memmap.amount; ++i) {
        if (memmap.entries[i].type == 3) {
            memmap.entries[i].type = 1;
        }
    }
}

void memmap_getMemorySizes(uint64* usable, uint64* reserved, uint64* acpi, uint64* nvs, uint64* bad)
{
    int i;

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
