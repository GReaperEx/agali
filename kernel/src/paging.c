#include "paging.h"
#include "memmap.h"

#define PAGE_STACK_SIZE 1024

static PageTableEntry globalPML4T[512] __attribute__((aligned(4096)));

static PageTableEntry kernelPDPT[512] __attribute__((aligned(4096)));
static PageTableEntry kernelPDT[512] __attribute__((aligned(4096)));
static PageEntry kernelPT[2*512] __attribute__((aligned(4096)));


static inline PageTableEntry* _getPML4entry(void* vAddr)
{
    return (void*)(0xFFFFFF8000000000 | ((((intptr)vAddr >> 39) & 0x1FF) << 30));
}

static inline PageTableEntry* _getPDTentry(void* vAddr)
{
    return (void*)((intptr)_getPML4entry(vAddr) | ((((intptr)vAddr >> 30) & 0x1FF) << 21));
}

static inline PageTableEntry* _getPDentry(void* vAddr)
{
    return (void*)((intptr)_getPDTentry(vAddr) | ((((intptr)vAddr >> 21) & 0x1FF) << 12));
}

static inline PageEntry* _getPTentry(void* vAddr)
{
    return (void*)((intptr)_getPDentry(vAddr) | ((((intptr)vAddr >> 12) & 0x1FF) << 3));
}

void paging_init(void)
{
    uint64 i, j, kernelSize;

    kernelSize = *((uint32*)0x7C00);

    globalPML4T[510].rvalue = ((intptr)_getPTentry(kernelPDPT)->pAddr << 12) | 0x3;
    globalPML4T[511].rvalue = ((intptr)_getPTentry(globalPML4T)->pAddr << 12) | 0x3;

    kernelPDPT[0].rvalue = ((intptr)_getPTentry(kernelPDT)->pAddr << 12) | 0x3;

    kernelPDT[0].rvalue = ((intptr)_getPTentry(&kernelPT[0])->pAddr << 12) | 0x3;
    kernelPDT[1].rvalue = ((intptr)_getPTentry(&kernelPT[512])->pAddr << 12) | 0x3;

    // Fetching the page entries from the temporary table
    for (i = 0, j = 0xFFFFFF0000000000; i < kernelSize; ++i, j += 4096) {
        kernelPT[i] = *_getPTentry((void*)j);
        kernelPT[i].G = 1;
    }

    // Enabling global pages
    __asm__ __volatile__(
        "mov %%cr4, %%rax \n\t"
        "or $0x80, %%rax \n\t"
        "mov %%rax, %%cr4 \n\t"
    ::: "%eax", "memory"
    );

    reloadCR3();
}

void* paging_getPhysAddr(void* vAddr)
{
    if (_getPML4entry(vAddr)->P) {
        if (_getPDTentry(vAddr)->P) {
            if (_getPDentry(vAddr)->P) {
                PageEntry* entry = _getPTentry(vAddr);
                if (entry->P) {
                    return (void*)(((uint64)entry->pAddr << 12) | ((intptr)vAddr & 0xFFF));
                }
            }
        }
    }
    return NULL;
}

void reloadCR3(void)
{
    void* address = paging_getPhysAddr(globalPML4T);

    __asm__ __volatile__("mov %0, %%cr3 \n\t" ::"r" (address) : "memory");
}
