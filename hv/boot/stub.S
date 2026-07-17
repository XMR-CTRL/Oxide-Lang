


    .section .multiboot, "ax"
    .set MB2_MAGIC, 0xE85250D6
    .align 8
header_start:
    .long MB2_MAGIC
    .long 0
    .long header_end - header_start
    .long 0x17ADAF12

    .short 0
    .short 0
    .long 8
header_end:


    .section .bss
    .align 4096
pml4:
    .skip 4096
pdpt:
    .skip 4096
pd:
    .skip 4096
    .align 16
    .global stack_top
stack_bottom:
    .skip 16384
stack_top:

    .section .text
    .code32
    .global _start
_start:
    cli


    mov $pdpt, %ecx
    or  $3, %ecx
    mov $pml4, %eax
    mov %ecx, (%eax)


    mov $pd, %ecx
    or  $3, %ecx
    mov $pdpt, %eax
    mov %ecx, (%eax)


    mov $0x83, %ecx
    mov $pd, %eax
    mov %ecx, (%eax)


    mov %cr4, %eax
    or  $(1 << 5), %eax
    mov %eax, %cr4


    mov $pml4, %eax
    mov %eax, %cr3


    mov $0xC0000080, %ecx
    rdmsr
    or  $(1 << 8), %eax
    wrmsr


    mov %cr0, %eax
    or  $(1 << 31), %eax
    mov %eax, %cr0


    lgdt gdtr64
    ljmp $0x08, $oxide_long_mode_entry

    .align 8
gdt64:
    .quad 0x0000000000000000
    .quad 0x00209A0000000000
    .quad 0x0000920000000000


    .quad 0x0000890000000068
    .quad 0x0000000000000000
gdt64_end:
gdtr64:
    .word gdt64_end - gdt64 - 1
    .long gdt64

    .align 16
    .global tss64
tss64:
    .skip 0x68
tss64_end:

    .section .note.GNU-stack,"",@progbits
