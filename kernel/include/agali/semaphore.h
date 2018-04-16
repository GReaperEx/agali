#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <agali/ktypes.h>
#include <agali/spinlock.h>

typedef struct {
    spinlock lock;

    uint16 curAcquires;
    uint16 maxAcquiresAllowed;

    uint64 uniqueID;
    BOOL locked;
} sema4;


void sema4_prepare(volatile sema4* sema, uint16 maxAcquires, BOOL startLocked);

void sema4_acquire(volatile sema4* sema);
void sema4_release(volatile sema4* sema);
void sema4_lock(volatile sema4* sema);
void sema4_unlock(volatile sema4* sema);

#endif // SEMAPHORE_H
