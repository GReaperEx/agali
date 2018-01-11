#ifndef PAGING_H
#define PAGING_H

#include "ktypes.h"

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

void paging_alloc(void* vAddr, void* pAddr, BOOL isSuper, BOOL isWritable);
void paging_free(void* vAddr, BOOL freePhysical);

#endif // PAGING_H