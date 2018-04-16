#ifndef REGISTERS_H
#define REGISTERS_H

#include <agali/ktypes.h>

typedef union {
    struct {
        uint64 CF    : 1; // Carry
        uint64 _res1 : 1; // 1
        uint64 PF    : 1; // Parity
        uint64 _res2 : 1; // 0
        uint64 AF    : 1; // Auxiliary Carry
        uint64 _res3 : 1; // 0
        uint64 ZF    : 1; // Zero
        uint64 SF    : 1; // Sign
        uint64 TF    : 1; // Trap
        uint64 IF    : 1; // Interrupt enable
        uint64 DF    : 1; // Direction
        uint64 OF    : 1; // Overflow
        uint64 IOPL  : 2; // I/O Privilege Level
        uint64 NT    : 1; // Nested Task
        uint64 _res4 : 1; // 0
        uint64 RF    : 1; // Resume
        uint64 VM    : 1; // Virtual-8086 Mode
        uint64 AC    : 1; // Alignment Check / Access Control
        uint64 VIF   : 1; // Virtual Interrupt flag
        uint64 VIP   : 1; // Virtual Interrupt Pending
        uint64 ID    : 1;
    };
    uint32 lvalue;
    uint64 rvalue;
} RFLAGS;

typedef union {
    struct {
        uint64 PE    :  1; // Protected mode Enable
        uint64 MP    :  1; // Monitor co-Processor
        uint64 EM    :  1; // Emulation
        uint64 TS    :  1; // Task Switched
        uint64 ET    :  1; // Extension Type
        uint64 NE    :  1; // Numeric Error
        uint64 _res1 : 10; // 0
        uint64 WP    :  1; // Write Protect
        uint64 _res2 :  1; // 0
        uint64 AM    :  1; // Alignment Mask
        uint64 _res3 : 10; // 0
        uint64 NW    :  1; // Not-Write through
        uint64 CD    :  1; // Cache Disable
        uint64 PG    :  1; // Paging Enable
    };
    uint32 lvalue;
    uint64 rvalue;
} CR0;

typedef union {
    uint32 lvalue;
    uint64 rvalue;
} CR2;

typedef union {
    uint32 lvalue;
    uint64 rvalue;
} CR3;

typedef union {
    struct {
        uint64 VME        : 1; // Virtual-8086 Mode Extensions
        uint64 PVI        : 1; // Protected mode Virtual Interrupts
        uint64 TSD        : 1; // ring 3 Time Stamp Disable
        uint64 DE         : 1; // Debugging Extensions
        uint64 PSE        : 1; // Page Size Extensions
        uint64 PAE        : 1; // Physical Address Extensions
        uint64 MCE        : 1; // Machine Check Exception
        uint64 PGE        : 1; // Page Global Enable
        uint64 PCE        : 1; // Performance monitoring Counter Enable
        uint64 OSFXSR     : 1; // OS supports fxsave/fxrstor
        uint64 OSXMMEXCPT : 1; // OS supports unmasked SIMD FP exceptions
        uint64 UIMP       : 1; // User-Mode Instruction Prevention
        uint64 _res1      : 1; // 0
        uint64 VMXE       : 1; // Virtual Machine Extensions Enable
        uint64 SMXE       : 1; // Safer Mode Extensions Enable
        uint64 _res2      : 1; // 0
        uint64 PCIDE      : 1; // PCID Enable
        uint64 OSXSAVE    : 1; // XSAVE and processor extented states Enable
        uint64 _res3      : 1; // 0
        uint64 SMEP       : 1; // Supervisor Mode Executions Protection Enable
        uint64 SMAP       : 1; // Supervisor Mode Access Protection Enable
    };
    uint32 lvalue;
    uint64 rvalue;
} CR4;

typedef union {
    struct {
        uint64 Priority : 3;
    };
    uint32 lvalue;
    uint64 rvalue;
} CR8;

typedef union {
    struct {
        uint64 SCE   : 1; // System Call Extensions
        uint64 _res  : 7; // 0
        uint64 LME   : 1; // Long Mode Enable
        uint64 LMA   : 1; // Long Mode Active
        uint64 NXE   : 1; // No-Execute Enable
        uint64 SVME  : 1; // Secure Virtual Machine Enable
        uint64 LMSLE : 1; // Long Mode Segment Limit Enable
        uint64 FFXSR : 1; // Fast FXSAVE/FXRSTOR
        uint64 TCE   : 1; // Translation Cache Extension
    };
    uint32 lvalue;
    uint64 rvalue;
} EFER;

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

    RFLAGS flags;

    uint64 RSP;
    uint64 SS;
} StackState;

static inline CR0 getCR0(void)
{
    CR0 local;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(local.rvalue));
    return local;
}

static inline CR2 getCR2(void)
{
    CR2 local;
    __asm__ __volatile__("mov %%cr2, %0" : "=r"(local.rvalue));
    return local;
}

static inline CR3 getCR3(void)
{
    CR3 local;
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(local.rvalue));
    return local;
}

static inline CR4 getCR4(void)
{
    CR4 local;
    __asm__ __volatile__("mov %%cr4, %0" : "=r"(local.rvalue));
    return local;
}

static inline CR8 getCR8(void)
{
    CR8 local;
    __asm__ __volatile__("mov %%cr8, %0" : "=r"(local.rvalue));
    return local;
}

static inline void setCR0(CR0 newVal)
{
    __asm__ __volatile__("mov %0, %%cr0" :: "r"(newVal.rvalue));
}

static inline void setCR2(CR2 newVal)
{
    __asm__ __volatile__("mov %0, %%cr2" :: "r"(newVal.rvalue));
}

static inline void setCR3(CR3 newVal)
{
    __asm__ __volatile__("mov %0, %%cr3" :: "r"(newVal.rvalue));
}

static inline void setCR4(CR4 newVal)
{
    __asm__ __volatile__("mov %0, %%cr4" :: "r"(newVal.rvalue));
}

static inline void setCR8(CR8 newVal)
{
    __asm__ __volatile__("mov %0, %%cr8" :: "r"(newVal.rvalue));
}

static inline uint64 getMSR(uint32 msr)
{
    uint32 low, high;
    __asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64)high << 32) | low;
}

static inline void setMSR(uint32 msr, uint64 value)
{
    uint32 low, high;

    low = (uint32)value;
    high = (uint32)(value >> 32);
    __asm__ __volatile__("wrmsr" :: "a"(low), "d"(high), "c"(msr));
}

#endif // REGISTERS_H
