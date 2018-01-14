#include "exceptions.h"
#include "descriptors.h"
#include "textui.h"
#include "kstring.h"
#include "paging.h"

typedef struct {
    uint64 RAX;
    uint64 RCX;
    uint64 RDX;
    uint64 RBX;
    uint64 __useless;
    uint64 RBP;
    uint64 RSI;
    uint64 RDI;
    uint64 R8;
    uint64 R9;
    uint64 R10;
    uint64 R11;
    uint64 R12;
    uint64 R13;
    uint64 R14;
    uint64 R15;

    uint64 RIP;
    uint64 CS;

    uint64 flags;

    uint64 RSP;
    uint64 SS;
} StackState;

typedef struct {
    uint64 RAX;
    uint64 RCX;
    uint64 RDX;
    uint64 RBX;
    uint64 __useless;
    uint64 RBP;
    uint64 RSI;
    uint64 RDI;
    uint64 R8;
    uint64 R9;
    uint64 R10;
    uint64 R11;
    uint64 R12;
    uint64 R13;
    uint64 R14;
    uint64 R15;

    uint64 errorCode;

    uint64 RIP;
    uint64 CS;

    uint64 flags;

    uint64 RSP;
    uint64 SS;
} StackStateEx;

void ISR0(void);
void ISR1(void);
void ISR2(void);
void ISR3(void);
void ISR4(void);
void ISR5(void);
void ISR6(void);
void ISR7(void);
void ISR8(void);
void ISR10(void);
void ISR11(void);
void ISR12(void);
void ISR13(void);
void ISR14(void);
void ISR16(void);
void ISR17(void);
void ISR18(void);
void ISR19(void);
void ISR20(void);
void ISR30(void);

void exceptions_init(void)
{
    idt_enable(0, (intptr)ISR0, 0x08, GATE_INT, TRUE);
    idt_enable(1, (intptr)ISR1, 0x08, GATE_INT, TRUE);
    idt_enable(2, (intptr)ISR2, 0x08, GATE_INT, TRUE);
    idt_enable(3, (intptr)ISR3, 0x08, GATE_INT, TRUE);
    idt_enable(4, (intptr)ISR4, 0x08, GATE_INT, TRUE);
    idt_enable(5, (intptr)ISR5, 0x08, GATE_INT, TRUE);
    idt_enable(6, (intptr)ISR6, 0x08, GATE_INT, TRUE);
    idt_enable(7, (intptr)ISR7, 0x08, GATE_INT, TRUE);
    idt_enable(8, (intptr)ISR8, 0x08, GATE_INT, TRUE);
    idt_enable(10, (intptr)ISR10, 0x08, GATE_INT, TRUE);
    idt_enable(11, (intptr)ISR11, 0x08, GATE_INT, TRUE);
    idt_enable(12, (intptr)ISR12, 0x08, GATE_INT, TRUE);
    idt_enable(13, (intptr)ISR13, 0x08, GATE_INT, TRUE);
    idt_enable(14, (intptr)ISR14, 0x08, GATE_INT, TRUE);
    idt_enable(16, (intptr)ISR16, 0x08, GATE_INT, TRUE);
    idt_enable(17, (intptr)ISR17, 0x08, GATE_INT, TRUE);
    idt_enable(18, (intptr)ISR18, 0x08, GATE_INT, TRUE);
    idt_enable(18, (intptr)ISR19, 0x08, GATE_INT, TRUE);
    idt_enable(20, (intptr)ISR20, 0x08, GATE_INT, TRUE);
    idt_enable(30, (intptr)ISR30, 0x08, GATE_INT, TRUE);
}

static void dumpRegisters(StackState* state);
static void _hang(void);

void divByZero(StackState* state)
{
    char buffer[32];

    textui_puts("Divide-by-zero error!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void debug(StackState* state)
{
    char buffer[32];

    textui_puts("Debugging not yet implemented!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void NMI(StackState* state)
{
    textui_puts("A non-maskable interrupt happened!\n");
    _hang();
}

void breakpoint(StackState* state)
{
    char buffer[32];

    textui_puts("Breakpoints not yet implemented!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void overflow(StackState* state)
{
    char buffer[32];

    textui_puts("Overflow occurred!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void boundExceeded(StackState* state)
{
    char buffer[32];

    textui_puts("Bound range exceeded!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void invalidOpcode(StackState* state)
{
    char buffer[32];

    textui_puts("Encountered an invalid opcode!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void devNotAvail(StackState* state)
{
    char buffer[32];

    textui_puts("FPU not available!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void doubleFault(StackState* state)
{
    char buffer[32];

    textui_puts("Double fault! Something is awfully wrong!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void invalidTSS(StackStateEx* state)
{
    char buffer[32];

    textui_puts("Invalid TSS!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters((StackState*)state);
    _hang();
}

void segNotPresent(StackStateEx* state)
{
    char buffer[32];

    textui_puts("Segment not present!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters((StackState*)state);
    _hang();
}

void stackSegFault(StackStateEx* state)
{
    char buffer[32];

    textui_puts("Stack segment fault!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters((StackState*)state);
    _hang();
}

void generalProtection(StackStateEx* state)
{
    char buffer[32];

    textui_puts("General protection fault!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters((StackState*)state);
    _hang();
}

void pageFault(StackStateEx* state)
{
    char buffer[32];
    uint64 address;

    __asm__ __volatile__("mov %%cr2, %0 \n\t" : "=r"(address) );

    textui_puts("Page fault!\n");
    textui_puts("Caused by code at: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_puts("\nAt address: ");
    textui_puts(int2str(address, 16, buffer, sizeof(buffer)));
    textui_puts("\n\tPage was present: ");
    textui_puts((state->errorCode & 0x1) ? "Yes" : "No");
    textui_puts("\n\tWas caused by a write: ");
    textui_puts((state->errorCode & 0x2) ? "Yes" : "No");
    textui_puts("\n\tWas caused in user mode: ");
    textui_puts((state->errorCode & 0x4) ? "Yes" : "No");
    textui_puts("\n\tWas caused by an instruction fetch: ");
    textui_puts((state->errorCode & 0x10) ? "Yes" : "No");
    textui_putchar('\n');

    dumpRegisters((StackState*)state);
    _hang();
}

void x87FPUexcept(StackState* state)
{
    char buffer[32];

    textui_puts("x87 FPU exception!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void alignCheck(StackState* state)
{
    char buffer[32];

    textui_puts("Alignment check!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void machCheck(StackState* state)
{
    char buffer[32];

    textui_puts("Machine check!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void SIMD_FPUexcept(StackState* state)
{
    char buffer[32];

    textui_puts("SIMD FPU exception!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void virtualExcept(StackState* state)
{
    char buffer[32];

    textui_puts("Virtualization exception!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}

void securityExcept(StackState* state)
{
    char buffer[32];

    textui_puts("Security exception!\n");
    textui_puts("At address: ");
    textui_puts(int2str(state->RIP, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');

    dumpRegisters(state);
    _hang();
}


static void dumpRegisters(StackState* state)
{
    char buffer[32];

    textui_puts("\tRAX=");
    textui_puts(int2str(state->RAX, 16, buffer, sizeof(buffer)));
    textui_puts(" RCX=");
    textui_puts(int2str(state->RCX, 16, buffer, sizeof(buffer)));
    textui_puts(" RDX=");
    textui_puts(int2str(state->RDX, 16, buffer, sizeof(buffer)));
    textui_puts(" RBX=");
    textui_puts(int2str(state->RBX, 16, buffer, sizeof(buffer)));
    textui_puts("\n\tRSP=");
    textui_puts(int2str(state->RSP, 16, buffer, sizeof(buffer)));
    textui_puts(" RBP=");
    textui_puts(int2str(state->RBP, 16, buffer, sizeof(buffer)));
    textui_puts(" RSI=");
    textui_puts(int2str(state->RSI, 16, buffer, sizeof(buffer)));
    textui_puts(" RDI=");
    textui_puts(int2str(state->RDI, 16, buffer, sizeof(buffer)));
    textui_puts("\n\tR8=");
    textui_puts(int2str(state->R8, 16, buffer, sizeof(buffer)));
    textui_puts(" R9=");
    textui_puts(int2str(state->R9, 16, buffer, sizeof(buffer)));
    textui_puts(" R10=");
    textui_puts(int2str(state->R10, 16, buffer, sizeof(buffer)));
    textui_puts(" R11=");
    textui_puts(int2str(state->R11, 16, buffer, sizeof(buffer)));
    textui_puts("\n\tR12=");
    textui_puts(int2str(state->R12, 16, buffer, sizeof(buffer)));
    textui_puts(" R13=");
    textui_puts(int2str(state->R13, 16, buffer, sizeof(buffer)));
    textui_puts(" R14=");
    textui_puts(int2str(state->R14, 16, buffer, sizeof(buffer)));
    textui_puts(" R15=");
    textui_puts(int2str(state->R15, 16, buffer, sizeof(buffer)));
    textui_putchar('\n');
}

static void _hang(void)
{
    for (;;) {
        __asm__ __volatile__("hlt \n\t");
    }
}
