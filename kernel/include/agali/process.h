#ifndef PROCESS_H
#define PROCESS_H

#include <agali/ktypes.h>
#include <agali/registers.h>
#include <agali/semaphore.h>

#define PROCESS_INVALID_ID 0

typedef uint64 processID;

typedef enum {
    PROCESS_STATE_INVALID,
    PROCESS_STATE_RUNNABLE,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_SLEEPING,
    PROCESS_STATE_ABORTED,
    PROCESS_STATE_FINISHED
} ProcessState;

typedef enum {
    PROCESS_PRIORITY_LOWEST,
    PROCESS_PRIORITY_LOW,
    PROCESS_PRIORITY_NORMAL,
    PROCESS_PRIORITY_HIGH,
    PROCESS_PRIORITY_HIGHEST
} ProcessPriority;

typedef enum {
    PROCESS_ABORT_ERROR, PROCESS_ABORT_MEMORY
} ProcessError;

void process_abort(ProcessError reason);

typedef struct {
    processID pID;
    spinlock memMapLock;
    spinlock memAllocLock;
} ProcessData;

void process_init(void);

// If a process was waiting for that specific reason, make it RUNNABLE (Priorities may apply)
void process_bumpWaiting(uint64 waitReason);

// Allow a process to get its own ID
processID process_getID(void);

// Allow a process to suspend itself, for whatever reason
void process_wait(uint64 reason);

#endif // PROCESS_H
