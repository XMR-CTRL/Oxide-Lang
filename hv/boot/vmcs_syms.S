

    .section .text
    .globl ox_host_rip
    .globl ox_guest_rip
    .globl ox_host_stack_top_addr
    .globl ox_entry_stack_top_addr
    .globl vmexit_entry
    .type ox_host_rip, @function
ox_host_rip:


    leaq vmexit_entry(%rip), %rax
    retq
    .size ox_host_rip, .-ox_host_rip
    .type ox_guest_rip, @function
ox_guest_rip:
    leaq guest_payload(%rip), %rax
    retq
    .size ox_guest_rip, .-ox_guest_rip
    .type ox_host_stack_top_addr, @function
ox_host_stack_top_addr:
    leaq host_stack + 0x2000(%rip), %rax
    retq
    .size ox_host_stack_top_addr, .-ox_host_stack_top_addr
    .type ox_entry_stack_top_addr, @function
ox_entry_stack_top_addr:
    leaq stack_top(%rip), %rax
    retq
    .size ox_entry_stack_top_addr, .-ox_entry_stack_top_addr


    .type vmexit_entry, @function
    .align 16
vmexit_entry:


    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, %rdi
    callq vmexit_c_handler
    testq %rax, %rax
    jnz vmexit_halt

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    vmresume

    leaq vmresume_failed(%rip), %rdi
    callq serial_puts
    callq halt_forever
    jmp vmexit_halt
vmexit_halt:

    addq $(15 * 8), %rsp
    callq vmexit_done
    callq halt_forever
    jmp vmexit_halt
    .size vmexit_entry, .-vmexit_entry

    .section .rodata
vmresume_failed:
    .asciz "[oxide-hv] vmresume FAILED (asm stub)\n"
    .section .text

    .section .note.GNU-stack,"",@progbits
