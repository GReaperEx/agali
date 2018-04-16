#include <agali/process.h>
#include <agali/kmalloc.h>

#include <string.h>

typedef struct __attribute__((aligned(16))) _ThreadState {
    StackState genRegs;
    uint8 fxsave_region[512] __attribute__((aligned(16)));

    ProcessState threadState;
    ProcessPriority threadPrior;

    struct _ThreadState* prev;
    struct _ThreadState* next;

    spinlock lock;
} ThreadState;

typedef struct _ProcIntState {
    processID pID;
    void* PML4TpAddr;
    ThreadState* threads;

    ProcessState procState;
    ProcessPriority procPrior;

    struct _ProcIntState* prev;
    struct _ProcIntState* next;

    spinlock lock;
} ProcIntState;

typedef struct {
    ProcIntState* activeProcess;
    ThreadState* activeThread;
} CoreState;

static int coreCount;
static CoreState* coreStates;

void process_init(void)
{
    // TODO: Parse the SMDT

    coreCount = 1;
    coreStates = kmalloc(coreCount*sizeof(CoreState));
    memset(coreStates, 0, coreCount*sizeof(CoreState));
}
