#include "paging.h"
#include "memmap.h"

#define PAGE_STACK_SIZE 1024

PageTableEntry globalPML4[512] __attribute__((aligned(4096)));
PageTableEntry userPDT[512] __attribute__((aligned(4096)));
PageTableEntry kernelPDT[512] __attribute__((aligned(4096)));

static inline PageTableEntry* _getMPL4entry(void* vAddr)
{
    intptr _vAddr = (intptr)vAddr;
    _vAddr &= 0xFFFFFF8000000000L;

    if (_vAddr == 0) {
        _vAddr = 0x0000007FFFFFFFF8L;
    } else {
        _vAddr = 0xFFFFFFFFFFFFFFF8L;
    }

    return (void*)_vAddr;
}

static inline PageTableEntry* _getPDTentry(void* vAddr)
{
    intptr _vAddr = (intptr)vAddr;
    _vAddr &= 0xFFFFFF8000000000L;

    if (_vAddr == 0) {
        _vAddr = 0x0000007FFFFFF000L & (((intptr)vAddr >> 27) & 0xFF8);
    } else {
        _vAddr = 0xFFFFFFFFFFFFF000L & (((intptr)vAddr >> 27) & 0xFF8);
    }

    return (void*)_vAddr;
}

static inline PageTableEntry* _getPDentry(void* vAddr)
{
    intptr _vAddr = (intptr)vAddr;
    _vAddr &= 0xFFFFFF8000000000L;

    if (_vAddr == 0) {
        _vAddr = 0x0000007FFFE00000L & (((intptr)vAddr >> 11) & 0x1FF000);
    } else {
        _vAddr = 0xFFFFFFFFFFE00000L & (((intptr)vAddr >> 11) & 0x1FF000);
    }

    return (void*)_vAddr;
}

static inline PageEntry* _getPTentry(void* vAddr)
{
    intptr _vAddr = (intptr)vAddr;
    _vAddr &= 0xFFFFFF8000000000L;

    if (_vAddr == 0) {
        _vAddr = 0x0000007FC0000000L & (((intptr)vAddr << 9) & 0x3FE00000);
    } else {
        _vAddr = 0xFFFFFFFFC0000000L & (((intptr)vAddr << 9) & 0x3FE00000);
    }

    return (void*)_vAddr;
}

void paging_init(void)
{
    int i;

    userPDT[0].rvalue = i*4096;
}
