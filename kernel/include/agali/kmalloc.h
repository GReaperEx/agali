#ifndef KMALLOC_H
#define KMALLOC_H

#include <agali/paging.h>

void* kmalloc(uint64 size);
void* krealloc(void* oldPtr, uint64 newSize);
void kfree(void* oldPtr);

#endif // KMALLOC_H
