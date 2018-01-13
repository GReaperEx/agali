#include "paging.h"
#include "memmap.h"

#define PAGE_STACK_SIZE 1024

static PageTableEntry globalPML4T[512] __attribute__((aligned(4096)));

static PageTableEntry kernelPDPT[512] __attribute__((aligned(4096)));
static PageTableEntry kernelPDT[512] __attribute__((aligned(4096)));
static PageEntry kernelPT[2*512] __attribute__((aligned(4096)));

static inline PageTableEntry* _getPML4entry(void* vAddr)
{
    return (PageTableEntry*)0xFFFFFFFFFFFFF000 + (((intptr)vAddr >> 39) & 0x1FF);
}

static inline PageTableEntry* _getPDTentry(void* vAddr)
{
    return (PageTableEntry*)0xFFFFFFFFFFE00000 + (((intptr)vAddr >> 30) & 0x3FFFF);
}

static inline PageTableEntry* _getPDentry(void* vAddr)
{
    return (PageTableEntry*)0xFFFFFFFFC0000000 + (((intptr)vAddr >> 21) & 0x7FFFFFF);
}

static inline PageEntry* _getPTentry(void* vAddr)
{
    return (PageEntry*)0xFFFFFF8000000000 + (((intptr)vAddr >> 12) & 0xFFFFFFFFF);
}

static struct {
    uint64 curSize;
    uint64 curIndex;
    uint64** base;
} pageStack;

static uint64 curMemBound;
static void* _nextAvailPage(void)
{
    uint64 upperBound = memmap_getMemUpperBound();
    void* pAddr = NULL;

    while (curMemBound < upperBound) {
        if (memmap_isMemoryUsable((void*)curMemBound, 4096)) {
            pAddr = (void*)curMemBound;
            curMemBound += 4096;
            break;
        } else {
            curMemBound += 4096;
        }
    }

    return pAddr;
}

void paging_init(void)
{
    uint64 i, j, k, kernelSize;

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

    pageStack.base = (void*)j;
    k = ((uint64)kernelPT[i-1].pAddr << 12) + 4096;
    // Allocate the remaining of the initial 4 MiB to the page stack
    for (; i < 1024; ++i, j += 4096, k += 4096) {
        kernelPT[i].rvalue = k | 0x3;
        kernelPT[i].G = 1;
        invlpg((void*)j);
    }
    pageStack.curSize = j - (uint64)pageStack.base;
    pageStack.curIndex = 0;

    curMemBound = k;

    // Identity paging the 1st MiB
    for (i = 0; i < 0x100000; i += 4096) {
        paging_map((void*)i, (void*)i, TRUE, TRUE);
    }
}

void* paging_getPhysAddr(void* vAddr)
{
    if (_getPML4entry(vAddr)->P && _getPDTentry(vAddr)->P && _getPDentry(vAddr)->P) {
        PageEntry* entry = _getPTentry(vAddr);
        if (entry->P) {
            return (void*)(((uint64)entry->pAddr << 12) | ((intptr)vAddr & 0xFFF));
        }
    }
    return NULL;
}

void reloadCR3(void)
{
    void* address = paging_getPhysAddr(globalPML4T);

    __asm__ __volatile__("mov %0, %%cr3 \n\t" ::"r" (address) : "memory");
}

void paging_map(void* vAddr, void* pAddr, BOOL isSuper, BOOL isWritable)
{
    PageTableEntry* pte;
    PageEntry* pe;
    int i;

    pte = _getPML4entry(vAddr);
    if (pte->P == 0) {
        if (pageStack.curIndex > 0) {
            pte->rvalue = (uint64)pageStack.base[--pageStack.curIndex] | 0x3;
        } else {
            void* newAddr = _nextAvailPage();
            if (newAddr == NULL) {
                // TODO: Implement some sort of page swapping or cleanup
            }
            pte->rvalue = (uint64)newAddr | 0x3;
        }
        invlpg(_getPDTentry(vAddr));

        PageTableEntry* newPDPT = (void*)((intptr)_getPDTentry(vAddr) & ~0xFFFUL);
        for (i = 0; i < 512; ++i) {
            newPDPT[i].rvalue = 0;
        }
    }

    pte = _getPDTentry(vAddr);
    if (pte->P == 0) {
        if (pageStack.curIndex > 0) {
            pte->rvalue = (uint64)pageStack.base[--pageStack.curIndex] | 0x3;
        } else {
            void* newAddr = _nextAvailPage();
            if (newAddr == NULL) {
                // TODO: Implement some sort of page swapping or cleanup
            }
            pte->rvalue = (uint64)newAddr | 0x3;
        }
        invlpg(_getPDentry(vAddr));

        PageTableEntry* newPDT = (void*)((intptr)_getPDentry(vAddr) & ~0xFFFUL);
        for (i = 0; i < 512; ++i) {
            newPDT[i].rvalue = 0;
        }
    }

    pte = _getPDentry(vAddr);
    if (pte->P == 0) {
        if (pageStack.curIndex > 0) {
            pte->rvalue = (uint64)pageStack.base[--pageStack.curIndex] | 0x3;
        } else {
            void* newAddr = _nextAvailPage();
            if (newAddr == NULL) {
                // TODO: Implement some sort of page swapping or cleanup
            }
            pte->rvalue = (uint64)newAddr | 0x3;
        }
        invlpg(_getPTentry(vAddr));

        PageEntry* newPT = (void*)((intptr)_getPTentry(vAddr) & ~0xFFFUL);
        for (i = 0; i < 512; ++i) {
            newPT[i].rvalue = 0;
        }
    }

    // Silently unmaps if the page is already mapped
    pe = _getPTentry(vAddr);
    if (pe->P == 1) {
        paging_unmap(vAddr, TRUE);
    }
    pe->rvalue = ((uint64)pAddr & 0x000FFFFFFFFFF000) | 0x1;
    pe->RW = (isWritable == TRUE);
    pe->US = (isSuper == FALSE);
    pe->G = (isSuper == TRUE);
    invlpg(vAddr);
}

void paging_unmap(void* vAddr, BOOL freePhysical)
{
    if (_getPML4entry(vAddr)->P && _getPDTentry(vAddr)->P && _getPDentry(vAddr)->P) {
        PageEntry* entry = _getPTentry(vAddr);
        if (entry->P) {
            entry->P = 0;
            if (freePhysical) {
                if (pageStack.curIndex*8 >= pageStack.curSize) {
                    paging_map((void*)((intptr)pageStack.base + pageStack.curSize), (void*)((uint64)entry->pAddr << 12), TRUE, TRUE);
                    pageStack.curSize += 4096;
                } else {
                    pageStack.base[pageStack.curIndex++] = (void*)((uint64)entry->pAddr << 12);
                    entry->rvalue = 0;
                    invlpg(vAddr);
                }
            }
        }
    }
}

void* paging_alloc(int amount, BOOL isSuper, BOOL isWritable)
{
}

void* paging_realloc(void* oldPtr, int oldAmount, int newAmount)
{
}

void paging_free(void* oldPtr, int oldAmount)
{
}
