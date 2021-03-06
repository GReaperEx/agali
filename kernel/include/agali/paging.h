#ifndef PAGING_H
#define PAGING_H

#include <agali/ktypes.h>

typedef union {
    struct {
        uint64 P     : 1;
        uint64 RW    : 1;
        uint64 US    : 1;
        uint64 PWT   : 1;
        uint64 PCD   : 1;
        uint64 A     : 1;
        uint64 _res1 : 6;
        uint64 pAddr : 40;
        uint64 _res2 : 11;
        uint64 XD    : 1;
    };
    uint64 rvalue;
} PageTableEntry;

typedef union {
    struct {
        uint64 P     : 1;
        uint64 RW    : 1;
        uint64 US    : 1;
        uint64 PWT   : 1;
        uint64 PCD   : 1;
        uint64 A     : 1;
        uint64 D     : 1;
        uint64 PAT   : 1;
        uint64 G     : 1;
        uint64 _res1 : 3;
        uint64 pAddr : 40;
        uint64 _res2 : 7;
        uint64 prkey : 4;
        uint64 XD    : 1;
    };
    uint64 rvalue;
} PageEntry;

void paging_init(void);

void* paging_getPhysAddr(void* vAddr);

void* paging_alloc(uint64 amount, BOOL isSuper, BOOL isWritable);
void* paging_realloc(void* oldPtr, uint64 oldAmount, uint64 newAmount, BOOL isSuper, BOOL isWritable);
void paging_free(void* oldPtr, uint64 oldAmount);

void paging_map(void* vAddr, void* pAddr, BOOL isSuper, BOOL isWritable);
void paging_unmap(void* vAddr, BOOL freePhysical);

void paging_enableCaching(void* vAddr);
void paging_disableCaching(void* vAddr);

static inline void invlpg(void* address)
{
    __asm__ __volatile__("invlpg (%0)" ::"r" (address) : "memory");
}

static inline void reloadCR3(void* PML4TpAddr)
{
    __asm__ __volatile__("mov %0, %%cr3 \n\t" ::"r" (PML4TpAddr) : "memory");
}

#endif // PAGING_H
