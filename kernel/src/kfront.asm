global kfront
extern kmain

kfront:
	MOV RSP, kernelStacks
	CALL kmain

section .bss

global kernelStacks
resb 0x8000
kernelStacks:

