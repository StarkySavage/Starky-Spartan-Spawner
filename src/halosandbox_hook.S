    # Halo Sandbox — Copyright (c) 2026 Ghost7134 — licence MIT.
    .text
    .globl halosandbox_dispatch_stub
    .extern halosandbox_on_main_thread
    .extern g_resume_rip
halosandbox_dispatch_stub:
    pushfq
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
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
    movq %rsp, %r12
    andq $-16, %rsp
    subq $0x320, %rsp
    # The dispatcher is inserted at an arbitrary instruction boundary, not at
    # a normal ABI call site. Preserve the complete x87/SSE state plus the
    # volatile upper halves of YMM0-15 so the interrupted Blam code resumes
    # bit-for-bit unchanged after the C handler returns.
    fxsave64 0x20(%rsp)
    vextractf128 $1, %ymm0, 0x220(%rsp)
    vextractf128 $1, %ymm1, 0x230(%rsp)
    vextractf128 $1, %ymm2, 0x240(%rsp)
    vextractf128 $1, %ymm3, 0x250(%rsp)
    vextractf128 $1, %ymm4, 0x260(%rsp)
    vextractf128 $1, %ymm5, 0x270(%rsp)
    vextractf128 $1, %ymm6, 0x280(%rsp)
    vextractf128 $1, %ymm7, 0x290(%rsp)
    vextractf128 $1, %ymm8, 0x2A0(%rsp)
    vextractf128 $1, %ymm9, 0x2B0(%rsp)
    vextractf128 $1, %ymm10, 0x2C0(%rsp)
    vextractf128 $1, %ymm11, 0x2D0(%rsp)
    vextractf128 $1, %ymm12, 0x2E0(%rsp)
    vextractf128 $1, %ymm13, 0x2F0(%rsp)
    vextractf128 $1, %ymm14, 0x300(%rsp)
    vextractf128 $1, %ymm15, 0x310(%rsp)
    call halosandbox_on_main_thread
    fxrstor64 0x20(%rsp)
    vinsertf128 $1, 0x220(%rsp), %ymm0, %ymm0
    vinsertf128 $1, 0x230(%rsp), %ymm1, %ymm1
    vinsertf128 $1, 0x240(%rsp), %ymm2, %ymm2
    vinsertf128 $1, 0x250(%rsp), %ymm3, %ymm3
    vinsertf128 $1, 0x260(%rsp), %ymm4, %ymm4
    vinsertf128 $1, 0x270(%rsp), %ymm5, %ymm5
    vinsertf128 $1, 0x280(%rsp), %ymm6, %ymm6
    vinsertf128 $1, 0x290(%rsp), %ymm7, %ymm7
    vinsertf128 $1, 0x2A0(%rsp), %ymm8, %ymm8
    vinsertf128 $1, 0x2B0(%rsp), %ymm9, %ymm9
    vinsertf128 $1, 0x2C0(%rsp), %ymm10, %ymm10
    vinsertf128 $1, 0x2D0(%rsp), %ymm11, %ymm11
    vinsertf128 $1, 0x2E0(%rsp), %ymm12, %ymm12
    vinsertf128 $1, 0x2F0(%rsp), %ymm13, %ymm13
    vinsertf128 $1, 0x300(%rsp), %ymm14, %ymm14
    vinsertf128 $1, 0x310(%rsp), %ymm15, %ymm15
    movq %r12, %rsp
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
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    popfq
    jmp *g_resume_rip(%rip)
