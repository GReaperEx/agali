global kfront
extern kmain

%macro PUSH_ALL 0
    SUB RSP, 128
    MOV [RSP + 0], RAX
    MOV [RSP + 8], RCX
    MOV [RSP + 16], RDX
    MOV [RSP + 24], RBX
    MOV [RSP + 40], RBP
    MOV [RSP + 48], RSI
    MOV [RSP + 56], RDI
    MOV [RSP + 64 + 0], R8
    MOV [RSP + 64 + 8], R9
    MOV [RSP + 64 + 16], R10
    MOV [RSP + 64 + 24], R11
    MOV [RSP + 64 + 32], R12
    MOV [RSP + 64 + 40], R13
    MOV [RSP + 64 + 48], R14
    MOV [RSP + 64 + 56], R15
%endmacro

%macro POP_ALL 0
    MOV RAX, [RSP + 0]
    MOV RCX, [RSP + 8]
    MOV RDX, [RSP + 16]
    MOV RBX, [RSP + 24]
    MOV RBP, [RSP + 40]
    MOV RSI, [RSP + 48]
    MOV RDI, [RSP + 56]
    MOV R8,  [RSP + 64 + 0]
    MOV R9,  [RSP + 64 + 8]
    MOV R10, [RSP + 64 + 16]
    MOV R11, [RSP + 64 + 24]
    MOV R12, [RSP + 64 + 32]
    MOV R13, [RSP + 64 + 40]
    MOV R14, [RSP + 64 + 48]
    MOV R15, [RSP + 64 + 56]
    ADD RSP, 128
%endmacro

kfront:
	MOV RSP, kernelStacks
	CALL kmain

global ISR0
extern divByZero
ISR0:
    PUSH_ALL

    MOV RDI, RSP
    CALL divByZero

    POP_ALL
    IRETQ

global ISR1
extern debug
ISR1:
    PUSH_ALL

    MOV RDI, RSP
    CALL debug

    POP_ALL
    IRETQ

global ISR2
extern NMI
ISR2:
    PUSH_ALL

    MOV RDI, RSP
    CALL NMI

    POP_ALL
    IRETQ

global ISR3
extern breakpoint
ISR3:
    PUSH_ALL

    MOV RDI, RSP
    CALL breakpoint

    POP_ALL
    IRETQ

global ISR4
extern overflow
ISR4:
    PUSH_ALL

    MOV RDI, RSP
    CALL overflow

    POP_ALL
    IRETQ

global ISR5
extern boundExceeded
ISR5:
    PUSH_ALL

    MOV RDI, RSP
    CALL boundExceeded

    POP_ALL
    IRETQ

global ISR6
extern invalidOpcode
ISR6:
    PUSH_ALL

    MOV RDI, RSP
    CALL invalidOpcode

    POP_ALL
    IRETQ

global ISR7
extern devNotAvail
ISR7:
    PUSH_ALL

    MOV RDI, RSP
    CALL devNotAvail

    POP_ALL
    IRETQ

global ISR8
extern doubleFault
ISR8:
    PUSH_ALL

    MOV RDI, RSP
    CALL doubleFault

    POP_ALL
    IRETQ

global ISR10
extern invalidTSS
ISR10:
    PUSH_ALL

    MOV RDI, RSP
    CALL invalidTSS

    POP_ALL
    ADD RSP, 8
    IRETQ

global ISR11
extern segNotPresent
ISR11:
    PUSH_ALL

    MOV RDI, RSP
    CALL segNotPresent

    POP_ALL
    ADD RSP, 8
    IRETQ

global ISR12
extern stackSegFault
ISR12:
    PUSH_ALL

    MOV RDI, RSP
    CALL stackSegFault

    POP_ALL
    ADD RSP, 8
    IRETQ

global ISR13
extern generalProtection
ISR13:
    PUSH_ALL

    MOV RDI, RSP
    CALL generalProtection

    POP_ALL
    ADD RSP, 8
    IRETQ

global ISR14
extern pageFault
ISR14:
    PUSH_ALL

    MOV RDI, RSP
    CALL pageFault

    POP_ALL
    ADD RSP, 8
    IRETQ

global ISR16
extern x87FPUexcept
ISR16:
    PUSH_ALL

    MOV RDI, RSP
    CALL x87FPUexcept

    POP_ALL
    IRETQ

global ISR17
extern alignCheck
ISR17:
    PUSH_ALL

    MOV RDI, RSP
    CALL alignCheck

    POP_ALL
    IRETQ

global ISR18
extern machCheck
ISR18:
    PUSH_ALL

    MOV RDI, RSP
    CALL machCheck

    POP_ALL
    IRETQ

global ISR19
extern SIMD_FPUexcept
ISR19:
    PUSH_ALL

    MOV RDI, RSP
    CALL SIMD_FPUexcept

    POP_ALL
    IRETQ

global ISR20
extern virtualExcept
ISR20:
    PUSH_ALL

    MOV RDI, RSP
    CALL virtualExcept

    POP_ALL
    IRETQ

global ISR30
extern securityExcept
ISR30:
    PUSH_ALL

    MOV RDI, RSP
    CALL securityExcept

    POP_ALL
    IRETQ

section .bss

global kernelStacks
resb 0x8000
kernelStacks:

