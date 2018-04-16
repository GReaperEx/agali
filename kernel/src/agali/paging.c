#include <agali/paging.h>
#include <agali/memmap.h>
#include <agali/process.h>
#include <agali/textui.h>


static PageTableEntry kernelPML4T[512] __attribute__((aligned(4096)));

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

static volatile struct {
    uint64 curSize;
    uint64 curIndex;
    uint64** base;
} pageStack;

static volatile uint64 curMemBound;
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

static void* _pageFromStack(void)
{
    void* pAddr = NULL;

    if (pageStack.curIndex > 0) {
        pAddr = pageStack.base[--pageStack.curIndex];
    }

    return pAddr;
}

static void _pageToStack(void* pAddr)
{
    if (pageStack.curIndex*8 >= pageStack.curSize) {
        void* vAddr = (void*)((intptr)pageStack.base + pageStack.curSize);
        void* pAddr2 = pageStack.base[--pageStack.curIndex];

        PageTableEntry* pte;
        PageEntry* pe;
        int i;

        pte = _getPML4entry(vAddr);
        if (pte->P == 0) {
            pte->rvalue = (uint64)pageStack.base[--pageStack.curIndex] | 0x3;
            invlpg(_getPDTentry(vAddr));

            PageTableEntry* newPDPT = (void*)((intptr)_getPDTentry(vAddr) & ~0xFFFUL);
            for (i = 0; i < 512; ++i) {
                newPDPT[i].rvalue = 0;
            }
        }

        pte = _getPDTentry(vAddr);
        if (pte->P == 0) {
            pte->rvalue = (uint64)pageStack.base[--pageStack.curIndex] | 0x3;
            invlpg(_getPDentry(vAddr));

            PageTableEntry* newPDT = (void*)((intptr)_getPDentry(vAddr) & ~0xFFFUL);
            for (i = 0; i < 512; ++i) {
                newPDT[i].rvalue = 0;
            }
        }

        pte = _getPDentry(vAddr);
        if (pte->P == 0) {
            pte->rvalue = (uint64)pageStack.base[--pageStack.curIndex] | 0x3;
            invlpg(_getPTentry(vAddr));

            PageEntry* newPT = (void*)((intptr)_getPTentry(vAddr) & ~0xFFFUL);
            for (i = 0; i < 512; ++i) {
                newPT[i].rvalue = 0;
            }
        }

        pe = _getPTentry(vAddr);
        pe->rvalue = ((uint64)pAddr2 & 0x000FFFFFFFFFF000) | 0x1;
        pe->RW = TRUE;
        pe->US = FALSE;
        pe->G = TRUE;
        invlpg(vAddr);

        pageStack.curSize += 4096;
    }
    pageStack.base[pageStack.curIndex++] = pAddr;
}

static void* _allocPhysicalPage(void)
{
    void *newPage = NULL;

    newPage = _pageFromStack();
    if (newPage == NULL) {
        newPage = _nextAvailPage();
    }
    if (newPage == NULL) {
        // TODO: Implement some sort of page swapping or cleanup
        textui_puts("FATAL: System is out of memory!\n");
        process_abort(PROCESS_ABORT_MEMORY);
    }

    return newPage;
}

void paging_init(void)
{
    uint64 i, j, k, kernelSize;

    kernelSize = *((uint32*)0x7C00);

    kernelPML4T[510].rvalue = ((intptr)_getPTentry(kernelPDPT)->pAddr << 12) | 0x3;
    kernelPML4T[511].rvalue = ((intptr)_getPTentry(kernelPML4T)->pAddr << 12) | 0x3;

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

    reloadCR3(paging_getPhysAddr(kernelPML4T));

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
    paging_map((void*)0x100000, _allocPhysicalPage(), TRUE, TRUE);
    // Bypass the flags for this particular page
    _getPTentry((void*)0x100000)->G = 0;
    invlpg((void*)0x100000);

    // Setting necessary data for the idle process
    ProcessData* data = (void*)0x100000;
    data->pID = 0;
    data->memMapLock = 0;
    data->memAllocLock = 0;
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

static volatile spinlock kernelMemMapLock;

void paging_map(void* vAddr, void* pAddr, BOOL isSuper, BOOL isWritable)
{
    PageTableEntry* pte;
    PageEntry* pe;
    int i;

    volatile spinlock* toAcquire;

    if (isSuper) {
        toAcquire = &kernelMemMapLock;
    } else {
        toAcquire = &(((ProcessData*)0x100000)->memMapLock);
    }

    spinlock_acquire(toAcquire);

    pte = _getPML4entry(vAddr);
    if (pte->P == 0) {
        pte->rvalue = (uint64)_allocPhysicalPage() | 0x3;
        invlpg(_getPDTentry(vAddr));

        PageTableEntry* newPDPT = (void*)((intptr)_getPDTentry(vAddr) & ~0xFFFUL);
        for (i = 0; i < 512; ++i) {
            newPDPT[i].rvalue = 0;
        }
    }

    pte = _getPDTentry(vAddr);
    if (pte->P == 0) {
        pte->rvalue = (uint64)_allocPhysicalPage() | 0x3;
        invlpg(_getPDentry(vAddr));

        PageTableEntry* newPDT = (void*)((intptr)_getPDentry(vAddr) & ~0xFFFUL);
        for (i = 0; i < 512; ++i) {
            newPDT[i].rvalue = 0;
        }
    }

    pte = _getPDentry(vAddr);
    if (pte->P == 0) {
        pte->rvalue = (uint64)_allocPhysicalPage() | 0x3;
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

    spinlock_release(toAcquire);
}

void paging_unmap(void* vAddr, BOOL freePhysical)
{
    volatile spinlock* toAcquire;
    BOOL isSuper = FALSE;
    if (_getPML4entry(vAddr)->P && _getPDTentry(vAddr)->P && _getPDentry(vAddr)->P) {
        isSuper = (_getPTentry(vAddr)->US == 0);
    }

    if (isSuper) {
        toAcquire = &kernelMemMapLock;
    } else {
        toAcquire = &(((ProcessData*)0x100000)->memMapLock);
    }

    spinlock_acquire(toAcquire);

    if (_getPML4entry(vAddr)->P && _getPDTentry(vAddr)->P && _getPDentry(vAddr)->P) {
        PageEntry* entry = _getPTentry(vAddr);
        if (entry->P) {
            entry->P = 0;
            if (freePhysical) {
                _pageToStack((void*)((uint64)entry->pAddr << 12));
                entry->rvalue = 0;
            }
            invlpg(vAddr);
        }
    }

    spinlock_release(toAcquire);
}

/* User memory is allocated conventionally, from 0x101000 to 0x8000000000 (0x100000 page contains process-specific info)
   Kernel memory is allocated from top to bottom, from 0xFFFFFF7FFFFFF000 to 0xFFFFFF0000000000 */

static volatile spinlock kernelMemAllocLock;

// Kernel memory is always global
void* paging_alloc(uint64 amount, BOOL isSuper, BOOL isWritable)
{
    uint64 lowBound, highBound;
    volatile spinlock* toAcquire = NULL;

    if (isSuper) {
        toAcquire = &kernelMemAllocLock;
    } else {
        toAcquire = &(((ProcessData*)0x100000)->memAllocLock);
    }

    if (amount == 0) {
        return NULL;
    }

    spinlock_acquire(toAcquire);

    lowBound = highBound = 0;
    if (isSuper) {
        uint64 kernelCounter = 0xFFFFFF8000000000;

        while(highBound - lowBound < amount*4096) {
            highBound = kernelCounter;
            for (;;) {
                highBound -= 4096;
                if (paging_getPhysAddr((void*)highBound) == NULL || highBound < 0xFFFFFF0000000000) {
                    break;
                }
            }

            lowBound = highBound;
            highBound += 4096;
            while (highBound - lowBound < amount*4096 && paging_getPhysAddr((void*)lowBound) == NULL && lowBound >= 0xFFFFFF0000000000) {
                lowBound -= 4096;
            }
            kernelCounter = lowBound;

            if (kernelCounter < 0xFFFFFF0000000000) {
                break;
            }
        }
    } else {
        uint64 userCounter = 0x101000;

        while(highBound - lowBound < amount*4096) {
            lowBound = userCounter;
            while (paging_getPhysAddr((void*)lowBound) != NULL && lowBound < 0x8000000000) {
                lowBound += 4096;
            }
            userCounter = lowBound;

            highBound = lowBound + 4096;
            while (highBound - lowBound < amount*4096 && paging_getPhysAddr((void*)highBound) == NULL && highBound < 0x8000000000) {
                highBound += 4096;
            }
            userCounter = highBound;

            if (userCounter >= 0x8000000000) {
                break;
            }
        }
    }

    if (highBound - lowBound == amount*4096) {
        void* address = (void*)lowBound;

        for (; lowBound < highBound; lowBound += 4096) {
            void* physical = NULL;

            physical = _allocPhysicalPage();

            paging_map((void*)lowBound, physical, isSuper, isWritable);
        }

        spinlock_release(toAcquire);
        return address;
    }

    spinlock_release(toAcquire);
    return NULL;
}

void* paging_realloc(void* oldPtr, uint64 oldAmount, uint64 newAmount, BOOL isSuper, BOOL isWritable)
{
    BOOL wasSuper, wasWritable;
    volatile spinlock* toAcquire = NULL;

    if (isSuper) {
        toAcquire = &kernelMemAllocLock;
    } else {
        toAcquire = &(((ProcessData*)0x100000)->memAllocLock);
    }

    if (newAmount == 0) {
        paging_free(oldPtr, oldAmount);
        return oldPtr;
    }
    if (oldPtr == NULL) {
        return paging_alloc(newAmount, isSuper, isWritable);
    }
    if (oldAmount == newAmount) {
        return oldPtr;
    }

    wasSuper = (_getPTentry(oldPtr)->US == FALSE);
    wasWritable = (_getPTentry(oldPtr)->RW == TRUE);

    if (isSuper != wasSuper || isWritable != wasWritable) {
        return NULL;
    }

    spinlock_acquire(toAcquire);

    if (newAmount < oldAmount) {
        intptr trimLow = (intptr)oldPtr + newAmount*4096;
        intptr trimHigh = (intptr)oldPtr + oldAmount*4096;
        for (; trimLow < trimHigh; trimLow += 4096) {
            paging_unmap((void*)trimLow, TRUE);
        }
    } else {
        uint64 i;
        void* newPtr = paging_alloc(newAmount, isSuper, isWritable);

        for (i = 0; i < oldAmount; ++i) {
            paging_map((void*)((intptr)newPtr + i*4096),
                       (void*)((uint64)_getPTentry((void*)((intptr)oldPtr + i*4096))->pAddr << 12),
                       isSuper, isWritable);
            paging_unmap((void*)((intptr)oldPtr + i*4096), FALSE);
        }
        oldPtr = newPtr;
    }

    spinlock_release(toAcquire);

    return oldPtr;
}

void paging_free(void* oldPtr, uint64 oldAmount)
{
    volatile spinlock* toAcquire = NULL;

    if (oldPtr != NULL) {
        uint64 i;

        if (_getPTentry(oldPtr)->US == 0) {
            toAcquire = &kernelMemAllocLock;
        } else {
            toAcquire = &(((ProcessData*)0x100000)->memAllocLock);
        }

        spinlock_acquire(toAcquire);
        for (i = 0; i < oldAmount; ++i) {
            paging_unmap((void*)((intptr)oldPtr + i*4096), TRUE);
        }
        spinlock_release(toAcquire);
    }
}

void paging_enableCaching(void* vAddr)
{
    if (_getPML4entry(vAddr)->P && _getPDTentry(vAddr)->P && _getPDentry(vAddr)->P) {
        PageEntry* pe = _getPTentry(vAddr);
        if (pe->P) {
            pe->PCD = pe->PWT = pe->PAT = 0;
            invlpg(vAddr);
        }
    }
}

void paging_disableCaching(void* vAddr)
{
    if (_getPML4entry(vAddr)->P && _getPDTentry(vAddr)->P && _getPDentry(vAddr)->P) {
        PageEntry* pe = _getPTentry(vAddr);
        if (pe->P) {
            pe->PCD = pe->PWT = pe->PAT = 1;
            invlpg(vAddr);
        }
    }
}
