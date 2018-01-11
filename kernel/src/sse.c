#include "sse.h"

static BOOL sse_enabled;

void sse_enable(void)
{
    uint64 cr;

    __asm__ __volatile__(
        "mov %%cr0, %0 \n\t"
    : "=r" (cr)
    );
    cr &= ~(1 << 2);
    cr |= (1 << 2);
    __asm__ __volatile__(
        "mov %0, %%cr0 \n\t"
    :: "r" (cr)
    );

    __asm__ __volatile__(
        "mov %%cr4, %0 \n\t"
    : "=r" (cr)
    );
    cr |= (1 << 9) | (1 << 10);
    __asm__ __volatile__(
        "mov %0, %%cr4 \n\t"
    :: "r" (cr)
    );

    sse_enabled = TRUE;
}

BOOL sse_isEnabled(void)
{
    return sse_enabled;
}
