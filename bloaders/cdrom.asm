[ORG 0x7C00]
[BITS 16]
bLoaderStart:
	JMP 0x0:boot
	times 8-($-$$) db 0
	
bi_PrimaryVolumeDescriptor  dd  0
bi_BootFileLocation         dd  0
bi_BootFileLength           dd  0
bi_Checksum                 dd  0
bi_Reserved                 resb  40
  
boot:
	XOR AX, AX
	MOV DS, AX
	MOV SS, AX
	MOV ES, AX
	MOV SP, 0x7000

	MOV [diskID], DL
; Loading any extra bootloader sectors
	MOV CX, (bLoaderEnd - bLoaderStart - 1)/2048
	OR CX, CX
	JZ loaderLoaded
	PUSH DWORD 0
	MOV EAX, [bi_BootFileLocation]
	ADD EAX, 1
	PUSH EAX
	PUSH WORD $$+2048
	PUSH CX
	CALL loadSectorLBA
	OR AX, AX
	JNZ loaderLoaded
	PUSH WORD [msgDiskError]
	CALL printStr
	PUSH WORD [msgReboot]
	CALL printStr
failLoop:
	HLT
	JMP failLoop

loaderLoaded:
; Enabling the A20 line
	CALL check_a20
	OR AX, AX
	JNZ A20_done
	
	CALL enable_A20_bios
	CALL check_a20
	OR AX, AX
	JNZ A20_done
	
	CALL enable_A20_kbrd
	MOV CX, 100
.test_kbrd:
	CALL check_a20
	OR AX, AX
	JNZ A20_done
	LOOP .test_kbrd
	
	CALL enable_A20_fast
.test_fast:
	CALL check_a20
	OR AX, AX
	JNZ A20_done
	LOOP .test_fast
	
	PUSH WORD [msgA20Error]
	CALL printStr
	PUSH WORD [msgReboot]
	CALL printStr
	JMP failLoop
	
A20_done:
; Notifying the BIOS that we ultimately want long mode
	MOV AX, 0xEC00
	MOV BL, 2
	INT 0x15

; Enabling "Unreal mode"
	CLI
	PUSH DS
	LGDT [gdtinfo]
	
	MOV EAX, CR0
	OR AL, 1
	MOV CR0, EAX
	JMP 0x8:.pmode
.pmode:
	MOV DX, 0x10
	MOV DS, DX
	
	AND AL, 0xFE
	MOV CR0, EAX
	JMP 0x0:.huge_unreal
.huge_unreal:
	POP DS
	STI

; Loading the kernel
	PUSH WORD msgLoading
	CALL printStr
	
	; Searching for the Primary Volume Record
	PUSH DWORD 0
	PUSH DWORD [bi_PrimaryVolumeDescriptor]
	PUSH WORD 0x7000
	PUSH WORD 1
	CALL loadSectorLBA
	OR AX, AX
	JZ kernelFail
	
	; Search for the "/boot" directory
	PUSH DWORD 4
	PUSH DWORD kernelDir1
	PUSH DWORD [0x7000 + 156 + 10]
	PUSH DWORD [0x7000 + 156 + 2]
	CALL loadCDdir
	CMP EAX, 0xFFFFFFFF
	JE kernelFail
	
	; Search for the "/boot/kernel" directory
	PUSH DWORD 6
	PUSH DWORD kernelDir2
	PUSH DWORD [EAX + 0x7000 + 10]
	PUSH DWORD [EAX + 0x7000 + 2]
	CALL loadCDdir
	CMP EAX, 0xFFFFFFFF
	JE kernelFail
	
	; Try to find the file "/boot/kernel/os.sys"
	PUSH DWORD 8
	PUSH DWORD kernelName
	PUSH DWORD [EAX + 0x7000 + 10]
	PUSH DWORD [EAX + 0x7000 + 2]
	CALL loadCDdir
	CMP EAX, 0xFFFFFFFF
	JE kernelFail
	
	; Loading the kernel to memory
	
	MOV ECX, [EAX + 0x7000 + 10]
	MOV EDX, [EAX + 0x7000 + 2]
	MOV [kernelSize], ECX
	MOV EDI, 0x100000
.kernelLoadLoop:
	PUSH DWORD 0
	PUSH EDX
	PUSH WORD 0x7000
	PUSH WORD 1
	CALL loadSectorLBA
	OR AX, AX
	JZ kernelFail
	
	PUSH DWORD 2048
	PUSH DWORD 0x7000
	PUSH EDI
	CALL memcpy
	OR AX, AX
	JZ .loadDone	
	ADD EDI, 2048
	INC EDX
	SUB ECX, 2048
	JG .kernelLoadLoop
	
	JMP kernelFail
.loadDone:
	
	PUSH WORD msgMemory
	CALL printStr

; Detecting memory
	
	MOV DI, 0x500
	XOR EAX, EAX
	MOV [DI], EAX
	MOV [DI + 4], EAX
	ADD DI, 8
	MOV EAX, 1
	MOV [DI + 20], EAX
	XOR EBX, EBX
	MOV EDX, 0x534D4150
	MOV EAX, 0xE820
	MOV ECX, 24
	INT 0x15
	JC memFail
	CMP EAX, 0x534D4150
	JNE memFail
.memLoop:
	ADD DI, 24
	MOV DWORD [DI + 20], 0x1
	INC DWORD [0x500]
	MOV EAX, 0xE820
	MOV ECX, 24
	INT 0x15
	JC .memDone
	OR EBX, EBX
	JZ .memDone
	JMP .memLoop
.memDone:
; Enabling 32-bit protected mode

	CLI
	LGDT [gdtinfo]
	
	MOV EAX, CR0
	OR AL, 1
	MOV CR0, EAX
	
	OR BYTE [flatCode + 6], 1 << 6
	
	JMP 0x08:loader32

kernelFail:
	PUSH WORD msgLoadError
	CALL printStr
	PUSH WORD msgReboot
	CALL printStr
	JMP failLoop

memFail:
	PUSH WORD msgMemError
	CALL printStr
	PUSH WORD msgReboot
	CALL printStr
	JMP failLoop

; (dword:dirLBA, dword:dirSize, dword:name, dword:namesize)
loadCDdir:
	PUSH BP
	MOV BP, SP
	PUSH ECX
	PUSH EDX
	PUSH EBX
	PUSH EDI
	
	MOV ECX, [BP + 4]
	MOV EAX, [BP + 8]

	ADD EAX, 2047
	XOR EDX, EDX
	MOV EBX, 2048
	DIV EBX ; From byte size to sector size
	MOV EDX, EAX

.loadLoop:
	PUSH DWORD 0
	PUSH ECX
	PUSH WORD 0x7000
	PUSH WORD 1
	CALL loadSectorLBA
	OR AX, AX
	JZ .failure
	XOR EBX, EBX
.searchLoop:
	PUSH DWORD [BP + 16]
	LEA EAX, [EBX + 0x7000 + 33]
	PUSH EAX
	PUSH DWORD [BP + 12]
	CALL memcmp
	JZ .success
	MOVZX EDI, BYTE [EBX + 0x7000]
	ADD EBX, EDI
	CMP EBX, 2048
	JL .searchLoop
	INC ECX
	DEC EDX
	JNZ .loadLoop
.failure:
	MOV EAX, 0xFFFFFFFF
	JMP .done
.success:
	MOV EAX, EBX
.done:
	POP EDI
	POP EBX
	POP EDX
	POP ECX
	POP BP
	RET 16

; (word:count, word:address, dword:lowerlba, dword:upperlbs)
loadSectorLBA:
	PUSH BP
	MOV BP, SP
	PUSHA
	POP AX
	
	MOV  BYTE [diskAddressPacket + 0], 16
	MOV  BYTE [diskAddressPacket + 1], 0
	MOV AX, [BP + 4]
	MOV [diskAddressPacket + 2], AX
	MOV AX, [BP + 6]
	MOV [diskAddressPacket + 4], AX
	MOV  WORD [diskAddressPacket + 6], 0x0
	MOV EAX, [BP + 8]
	MOV [diskAddressPacket + 8], EAX
	MOV EAX, [BP + 12]
	MOV [diskAddressPacket + 12], EAX
	
	MOV SI, diskAddressPacket
	MOV AH, 0x42
	MOV DL, [diskID]
	INT 0x13
	JC .failure
	OR AH, AH
	JZ .success
	
.failure:
	XOR AX, AX
	JMP .done
.success:
	MOV AX, 1
.done:
	PUSH AX
	POPA
	POP BP
	RET 12

; ( word:address )
printStr:
	PUSH BP
	MOV BP, SP
	PUSHA
	
	MOV SI, [BP + 4]
.printLoop:
	LODSB
	OR AL, AL
	JZ .done
	MOV AH, 0xE
	INT 0x10
	JMP .printLoop
.done:

	POPA
	POP BP
	RET 2

; A20 functions taken from http://wiki.osdev.org/A20_Line

check_a20:
    pushf
    push ds
    push es
    push di
    push si
 
    cli
 
    xor ax, ax ; ax = 0
    mov es, ax
 
    not ax ; ax = 0xFFFF
    mov ds, ax
 
    mov di, 0x0500
    mov si, 0x0510
 
    mov al, byte [es:di]
    push ax
 
    mov al, byte [ds:si]
    push ax
 
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF
 
    cmp byte [es:di], 0xFF
 
    pop ax
    mov byte [ds:si], al
 
    pop ax
    mov byte [es:di], al
 
    mov ax, 0
    je check_a20__exit
 
    mov ax, 1
 
check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf
 
    ret

enable_A20_kbrd:
    cli
 
    call    a20wait
    mov     al,0xAD
    out     0x64,al

    call    a20wait
    mov     al,0xD0
    out     0x64,al

    call    a20wait2
    in      al,0x60
    push    eax

    call    a20wait
    mov     al,0xD1
    out     0x64,al

    call    a20wait
    pop     eax
    or      al,2
    out     0x60,al

    call    a20wait
    mov     al,0xAE
    out     0x64,al

    call    a20wait
    sti
    ret

a20wait:
    in      al,0x64
    test    al,2
    jnz     a20wait
    ret

 
a20wait2:
    in      al,0x64
    test    al,1
    jz      a20wait2
    ret

enable_A20_fast:
	IN AL, 0x92
	TEST AL, 2
	JNZ .after
	OR AL, 2
	AND AL, 0xFE
	OUT 0x92, AL
.after:
	RET

enable_A20_bios:
	mov     ax,2403h
	int     15h
	jb      .failed 
	cmp     ah,0
	jnz     .failed  
	 
	mov     ax,2402h
	int     15h
	jb      .failed
	cmp     ah,0
	jnz     .failed
	 
	cmp     al,1
	jz      .succeeded
	 
	mov     ax,2401h
	int     15h
	jb      .failed
	cmp     ah,0
	jnz     .failed

.succeeded:
	MOV AX, 1
	RET
.failed:
	XOR AX, AX
	RET

; (dword:dst, dword:src, dword:size)
memcpy:
	PUSH BP
	MOV BP, SP
	PUSH ESI
	PUSH EDI
	PUSH ECX
	
	MOV ESI, [BP + 4]
	MOV EDI, [BP + 8]
	MOV ECX, [BP + 12]
	CMP ECX, 0
	JLE .cpyDone
.cpyLoop:
	MOV AL, [EDI]
	MOV [ESI], AL
	INC ESI
	INC EDI
	DEC ECX
	JNZ .cpyLoop
.cpyDone:
	POP ECX
	POP EDI
	POP ESI
	POP BP
	RET 12

; (dword:src1, dword:src2, dword:size)
memcmp:
	PUSH BP
	MOV BP, SP
	PUSH ESI
	PUSH EDI
	PUSH ECX
	
	MOV ESI, [BP + 4]
	MOV EDI, [BP + 8]
	MOV ECX, [BP + 12]
	CMP ECX, 0
	JLE .cmpDone
.cmpLoop:
	MOV AL, [ESI]
	SUB AL, [EDI]
	INC ESI
	INC EDI
	OR AL, AL
	JNZ .cmpDone
	DEC ECX
	JNZ .cmpLoop
.cmpDone:
	POP ECX
	POP EDI
	POP ESI
	POP BP
	RET 12

diskID db 0
diskAddressPacket resb 16

kernelSize dd 0

gdtinfo:
   dw gdt_end - gdt - 1
   dd gdt
 
gdt         dd 0, 0
flatCode    db 0xFF, 0xFF, 0, 0, 0, 10011010b, 10001111b, 0
flatData    db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:

msgDiskError db "Unable to load bootloader!", 0xD, 0xA, 0x0
msgA20Error db "Cannot enable A20 line! Impossibru!!", 0xD, 0xA, 0x0
msgReboot db "Press Ctrl-Alt-Del to reboot.", 0xD, 0xA, 0x0

msgLoading db "Loading kernel.", 0xD, 0xA, 0x0
msgLoadError db "Failed to load the kernel!", 0xD, 0xA, 0x0

msgMemory db "Detecting available memory.", 0xD, 0xA, 0x0
msgMemError db "Failed at detecting memory!", 0xD, 0xA, 0x0

kernelDir1 db "BOOT"
kernelDir2 db "KERNEL"
kernelName db "OS.SYS;1"

;;;;;;;;;;;;; 32-bit Protected Mode starts here ;;;;;;;;;;;;;;;;;;;

[BITS 32]
loader32:
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV SS, AX
	MOV ESP, 0x80000

; Preparing the 64-bit paging tables

pagesStart equ ((bLoaderEnd + (4096 - ((bLoaderEnd - bLoaderStart + 0x7C00) % 4096)) % 4096) - bLoaderStart + 0x7C00)

	PUSH DWORD 4*4096
	PUSH DWORD pagesStart
	CALL memclear
	
	; Setting 1st PML4 entry
	MOV DWORD [pagesStart], (pagesStart + 4096) | 0x3
	MOV DWORD [pagesStart + 4], 0x0
	
	; Setting 1st PDT entry
	MOV DWORD [pagesStart + 4096], (pagesStart + 2*4096) | 0x3
	MOV DWORD [pagesStart + 4096 + 4], 0x0
	
	; Setting 1st PD entry
	MOV DWORD [pagesStart + 2*4096], (pagesStart + 3*4096) | 0x3
	MOV DWORD [pagesStart + 2*4096 + 4], 0x0
	
	; Identity paging the first MiB
	MOV ECX, 256
	MOV EDI, pagesStart + 3*4096
	XOR EAX, EAX
	XOR EDX, EDX
.identityLoop:
	MOV EBX, EAX
	OR EBX, 0x3
	MOV [EDI], EBX
	MOV [EDI + 4], EDX
	ADD EDI, 8
	ADD EAX, 4096
	DEC ECX
	JNZ .identityLoop
	
	CLI
	
	; Set CR4.PAE
	MOV EAX, CR4
	OR EAX, 1 << 5
	MOV CR4, EAX
	
	MOV EAX, pagesStart
	MOV CR3, EAX
	
	; Set EFER.LM
	MOV ECX, 0xC0000080
	RDMSR
	OR EAX, 1 << 8
	WRMSR
	
	; Enable paging
	MOV EAX, CR0
	OR EAX, 1 << 31
	MOV CR0, EAX
	
; Enabling long mode
	LGDT [GDT64.Pointer]
	JMP 0x08:loader64
	
; (*dst, size)
memclear:
	PUSH EBP
	MOV EBP, ESP
	PUSH ECX
	PUSH EDI
	
	XOR EAX, EAX
	MOV EDI, [EBP + 8]
	MOV ECX, [EBP + 12]
.clearLoop:
	MOV [EDI], EAX
	ADD EDI, 4
	SUB ECX, 4
	JG .clearLoop
	
	POP EDI
	POP ECX
	POP EBP
	RET 8

GDT64:
.null	dq 0
.code	db 0, 0, 0, 0, 0, 10011010b, 00100000b, 0
.data	db 0, 0, 0, 0, 0, 10010010b, 00000000b, 0
.Pointer:
    dw $ - GDT64 - 1
    dq GDT64

times 2046-($-$$) db 0
dw 0xAA55

;;;;;;;;;;;;; Long Mode starts here ;;;;;;;;;;;;;;;;;;;

[BITS 64]
loader64:
	CLI
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	
	; testing
	MOV EDI, 0xB8000
	MOV RAX, 0x1F201F201F201F20
	MOV ECX, 500
	REP STOSQ
	HLT

times (2048-($-$$) % 2048) % 2048 db 0
bLoaderEnd:
