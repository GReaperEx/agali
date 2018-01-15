#ifndef PORTIO_H
#define PORTIO_H

#include <agali/ktypes.h>

static inline uint8 inb(uint16 port)
{
    uint8 result;

    __asm__ __volatile__(
        "inb %%dx \n\t"
     :  "=a" (result)
     :  "d" (port)
    );

    return result;
}

static inline uint16 inw(uint16 port)
{
    uint16 result;

    __asm__ __volatile__(
        "inw %%dx \n\t"
     :  "=a" (result)
     :  "d" (port)
    );

    return result;
}

static inline uint32 ind(uint16 port)
{
    uint32 result;

    __asm__ __volatile__(
        "inl %%dx \n\t"
     :  "=a" (result)
     :  "d" (port)
    );

    return result;
}

static inline void outb(uint16 port, uint8 value)
{
    __asm__ __volatile__(
        "outb %%dx \n\t"
    ::  "a" (value), "d" (port)
    );
}

static inline void outw(uint16 port, uint8 value)
{
    __asm__ __volatile__(
        "outw %%dx \n\t"
    ::  "a" (value), "d" (port)
    );
}

static inline void outd(uint16 port, uint8 value)
{
    __asm__ __volatile__(
        "outl %%dx \n\t"
    ::  "a" (value), "d" (port)
    );
}

#endif // PORTIO_H
