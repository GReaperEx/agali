#ifndef SPINLOCK_H
#define SPINLOCK_H

typedef unsigned spinlock;

static inline void spinlock_acquire(volatile spinlock* lock)
{
    __asm__ __volatile__(
        "1:                  \n\t"
        "lock bts $0x0, (%0) \n\t"
        "jnc 3f              \n\t"
        "2:                  \n\t"
        "pause               \n\t"
        "testl $0x1, (%0)    \n\t"
        "jnz 2b              \n\t"
        "jmp 1b              \n\t"
        "3:                  \n\t"
     : "=r"(lock)
    );
}

static inline void spinlock_release(volatile spinlock* lock)
{
    *lock = 0;
}

#endif // SPINLOCK_H
