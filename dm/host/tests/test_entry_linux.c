#include <assert.h>
#include <stdint.h>

extern uint64_t dm_gate_top[1024];
static unsigned char gate_stack[1 << 20] __attribute__((aligned(16)));

uint64_t gate_dispatch(uint64_t ret, uint64_t *regs) {
    assert(ret != 0);
    assert(regs[0] == 7 && regs[1] == 11);
    regs[0] = 18;
    return 0;
}
void unimpl_c(unsigned index, uint64_t *regs) { (void)index; (void)regs; assert(0); }
void dm_gate_nested(uint64_t sp) { (void)sp; assert(0); }
void dm_thread_returned(void) { assert(0); }

int main(void) {
    dm_gate_top[0] = (uint64_t)(uintptr_t)(gate_stack + sizeof gate_stack);
    uint64_t result;
    __asm__ volatile(
        "mov x0, #7\n"
        "mov x1, #11\n"
        "stp x29, x30, [sp, #-16]!\n"
        "adr x30, 1f\n"
        "b dm_gate_stubs\n"
        "1: ldp x29, x30, [sp], #16\n"
        "mov %0, x0\n"
        : "=r"(result) : : "x0", "x1", "x29", "x30", "memory");
    assert(result == 18);
}
