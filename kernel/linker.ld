ENTRY(kfront)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    . = 0xFFFFFFFF80000000;
    
    .text BLOCK(4K) : ALIGN (4K)
    {
        *(.text)
    }
    
    .data BLOCK(4K) : ALIGN (4K)
    {
        *(.data)
        *(.rodata*)
        *(.eh_frame)
        *(.comment)
    }
    
    .bss BLOCK(4K) : ALIGN (4K)
    {
        *(COMMON)
        *(.bss)
        *(.sbss)
    }
}

