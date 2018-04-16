#include <agali/semaphore.h>
#include <agali/process.h>
#include <string.h>

static volatile uint64 uniqueIDpool;
static volatile spinlock uniqueIDpool_lock;

void sema4_prepare(volatile sema4* sema, uint16 maxAcquires, BOOL startLocked)
{
    memset(sema, 0, sizeof(sema4));

    sema->maxAcquiresAllowed = maxAcquires;
    sema->locked = startLocked;

    spinlock_acquire(&uniqueIDpool_lock);
    sema->uniqueID = ++uniqueIDpool;
    spinlock_release(&uniqueIDpool_lock);
}

void sema4_acquire(volatile sema4* sema)
{
    spinlock_acquire(&(sema->lock));

    while (sema->locked || sema->curAcquires >= sema->maxAcquiresAllowed) {
        spinlock_release(&(sema->lock));
        process_wait(sema->uniqueID);
        spinlock_acquire(&(sema->lock));
    }
    ++(sema->curAcquires);

    spinlock_release(&(sema->lock));
}

void sema4_release(volatile sema4* sema)
{
    spinlock_acquire(&(sema->lock));

    --(sema->curAcquires);
    process_bumpWaiting(sema->uniqueID);

    spinlock_release(&(sema->lock));
}

void sema4_lock(volatile sema4* sema)
{
    spinlock_acquire(&(sema->lock));

    while (sema->locked || sema->curAcquires != 0) {
        spinlock_release(&(sema->lock));
        process_wait(sema->uniqueID);
        spinlock_acquire(&(sema->lock));
    }
    sema->locked = TRUE;

    spinlock_release(&(sema->lock));
}

void sema4_unlock(volatile sema4* sema)
{
    spinlock_acquire(&(sema->lock));

    sema->locked = FALSE;
    process_bumpWaiting(sema->uniqueID);

    spinlock_release(&(sema->lock));
}
