#include "linux_tls.h"
#include <stdlib.h>

#ifndef DM_EXTERNAL_TLS_SLOTS
__thread uint64_t dm_linux_slots[4];
#endif

uint64_t dm_linux_slot_offset(unsigned slot) {
    if (slot >= 4) abort();
    uintptr_t tp;
    __asm__ volatile("mrs %0, tpidr_el0" : "=r"(tp));
    return (uintptr_t)&dm_linux_slots[slot] - tp;
}
