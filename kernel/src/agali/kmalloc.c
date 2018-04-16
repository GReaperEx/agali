#include <agali/kmalloc.h>
#include <agali/spinlock.h>

#include <string.h>

typedef struct _kmalloc_header {
    void* baseAddress;
    uint64 totalSize;

    uint64 chunkSize;
    struct _kmalloc_header* next;
} kmalloc_header;

static volatile spinlock kmalloc_lock;
static kmalloc_header* kmalloc_base;

static inline uint64 max(uint64 a, uint64 b)
{
    return a > b ? a : b;
}

void* kmalloc(uint64 size)
{
    void* toReturn = NULL;
    spinlock_acquire(&kmalloc_lock);

    size += sizeof(kmalloc_header);
    size += (16 - size % 16) % 16;

    if (kmalloc_base == NULL) {
        kmalloc_base = paging_alloc(0x100000/4096, TRUE, TRUE);

        kmalloc_base->baseAddress = kmalloc_base;
        kmalloc_base->totalSize = 0x100000;
        kmalloc_base->chunkSize = sizeof(kmalloc_header);
        kmalloc_base->next = NULL;

        toReturn = kmalloc_base + 1;
    }

    kmalloc_header* leaf = kmalloc_base;
    for (;;) {
        if (leaf->next != NULL) {
            if (leaf->next->baseAddress == leaf->baseAddress) {
                intptr top = (intptr)leaf + leaf->chunkSize;
                if ((intptr)(leaf->next) - top >= size) {
                    toReturn = (void*)top;
                    ((kmalloc_header*)toReturn)->baseAddress = leaf->baseAddress;
                    ((kmalloc_header*)toReturn)->totalSize = leaf->totalSize;
                    ((kmalloc_header*)toReturn)->chunkSize = size;
                    ((kmalloc_header*)toReturn)->next = leaf->next;
                    leaf->next = toReturn;
                    toReturn = (void*)((intptr)toReturn + sizeof(kmalloc_header));
                    break;
                }
            } else {
                intptr top = (intptr)leaf + leaf->chunkSize;
                if (((intptr)(leaf->baseAddress) + leaf->totalSize) - top >= size) {
                    toReturn = (void*)top;
                    ((kmalloc_header*)toReturn)->baseAddress = leaf->baseAddress;
                    ((kmalloc_header*)toReturn)->totalSize = leaf->totalSize;
                    ((kmalloc_header*)toReturn)->chunkSize = size;
                    ((kmalloc_header*)toReturn)->next = leaf->next;
                    leaf->next = toReturn;
                    toReturn = (void*)((intptr)toReturn + sizeof(kmalloc_header));
                    break;
                }
            }
        } else {
            intptr top = (intptr)leaf + leaf->chunkSize;
            if (((intptr)(leaf->baseAddress) + leaf->totalSize) - top >= size) {
                toReturn = (void*)top;
                ((kmalloc_header*)toReturn)->baseAddress = leaf->baseAddress;
                ((kmalloc_header*)toReturn)->totalSize = leaf->totalSize;
                ((kmalloc_header*)toReturn)->chunkSize = size;
                ((kmalloc_header*)toReturn)->next = NULL;
                leaf->next = toReturn;
                toReturn = (void*)((intptr)toReturn + sizeof(kmalloc_header));
                break;
            }

            uint64 maxSize = ((size + 0xFFFFF)/0x100000)*0x100000;

            /* No available memory, allocating new pages. Multiples of 1 MiB */
            leaf->next = paging_alloc(maxSize/4096, TRUE, TRUE);
            leaf->next->baseAddress = leaf->next;
            leaf->next->totalSize = maxSize;
            leaf->next->chunkSize = size;
            leaf->next->next = NULL;
            toReturn = (void*)((intptr)(leaf->next) + sizeof(kmalloc_header));
            break;
        }
        leaf = leaf->next;
    }

    spinlock_release(&kmalloc_lock);
    return toReturn;
}

void* krealloc(void* oldPtr, uint64 newSize)
{
    if (oldPtr == NULL) {
        return kmalloc(newSize);
    }

    newSize += sizeof(kmalloc_header);
    newSize = ((newSize + 15)/16)*16;

    spinlock_acquire(&kmalloc_lock);

    kmalloc_header* leaf = kmalloc_base;
    while (leaf != NULL) {
        if ((intptr)leaf + sizeof(kmalloc_header) == (intptr)oldPtr) {
            if (leaf->chunkSize > newSize) {
                leaf->chunkSize = newSize;
            } else {
                if ((leaf->next != NULL &&
                     leaf->next->baseAddress == leaf->baseAddress &&
                     (intptr)(leaf->next) - (intptr)leaf >= newSize) ||
                    ((intptr)(leaf->baseAddress) + leaf->totalSize) - (intptr)leaf >= newSize) {
                    leaf->chunkSize = newSize;
                    break;
                }

                void* newPtr;

                /* No available memory, allocating new pages */
                spinlock_release(&kmalloc_lock);

                newPtr = kmalloc(newSize);
                memcpy(newPtr, oldPtr, leaf->chunkSize - sizeof(kmalloc_header));
                kfree(oldPtr);

                return newPtr;
            }
            break;
        }
        leaf = leaf->next;
    }
    spinlock_release(&kmalloc_lock);

    return oldPtr;
}

void kfree(void* oldPtr)
{
    spinlock_acquire(&kmalloc_lock);
    if (oldPtr != NULL) {
        kmalloc_header* leaf = kmalloc_base;
        while (leaf != NULL) {
            if ((intptr)(leaf->next) + sizeof(kmalloc_header) == (intptr)oldPtr) {
                if (leaf->next->baseAddress != leaf->baseAddress &&
                    (leaf->next->next == NULL || leaf->next->next->baseAddress != leaf->baseAddress)) {
                    void* temp = leaf->next->next;
                    paging_free(leaf->next->baseAddress, leaf->next->totalSize/4096);
                    leaf->next = temp;
                } else {
                    leaf->next = leaf->next->next;
                }
                break;
            }
            leaf = leaf->next;
        }
    }
    spinlock_release(&kmalloc_lock);
}
