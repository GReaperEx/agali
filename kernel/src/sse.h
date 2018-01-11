#ifndef SSE_H
#define SSE_H

#include "ktypes.h"

void sse_enable(void);
BOOL sse_isEnabled(void);

typedef uint8 sse_vec8 __attribute__((vector_size(16)));
typedef uint16 sse_vec16 __attribute__((vector_size(16)));
typedef uint32 sse_vec32 __attribute__((vector_size(16)));
typedef uint64 sse_vec64 __attribute__((vector_size(16)));

#endif // SSE_H
