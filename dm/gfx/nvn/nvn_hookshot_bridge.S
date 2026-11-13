// Linux AArch64: x16 is the NxNative NVN slot. Hookshot's host NVN layer reads Ctx registers.
    .text
    .p2align 2
    .globl nvn_hookshot_entry
    .type nvn_hookshot_entry, %function
nvn_hookshot_entry:
    sub sp, sp, #864
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    str x30, [sp, #240]             // Ctx.x[30]
    add x9, sp, #864
    str x9, [sp, #248]              // Ctx.x[31], the guest SP
    stp q0, q1, [sp, #272]          // offsetof(Ctx, vreg)
    stp q2, q3, [sp, #304]
    stp q4, q5, [sp, #336]
    stp q6, q7, [sp, #368]
    mov x1, x16
    mov x0, sp
    adrp x16, nvn_hookshot_dispatch
    add x16, x16, :lo12:nvn_hookshot_dispatch
    bl dm_bigstack_enter
    ldp x0, x1, [sp, #0]
    ldp q0, q1, [sp, #272]
    ldp q2, q3, [sp, #304]
    ldr x30, [sp, #240]
    add sp, sp, #864
    ret
    .size nvn_hookshot_entry, .-nvn_hookshot_entry
