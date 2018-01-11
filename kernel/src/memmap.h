#ifndef MEMMAP_H
#define MEMMAP_H

#include "ktypes.h"

void memmap_init(void);

BOOL memmap_isMemoryUsable(void* address, uint64 size);

void memmap_reclaimACPI(void);

void memmap_getMemorySizes(uint64* usable, uint64* reserved, uint64* acpi, uint64* nvs, uint64* bad);

uint64 memmap_getMemUpperBound(void);

#endif // MEMMAP_H
