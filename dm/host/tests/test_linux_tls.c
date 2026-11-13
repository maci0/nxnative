#include "../linux_tls.h"
#include <assert.h>
#include <pthread.h>
#include <stdint.h>

static void check(unsigned slot) {
    uintptr_t tp;
    __asm__ volatile("mrs %0, tpidr_el0" : "=r"(tp));
    assert(tp + dm_linux_slot_offset(slot) == (uintptr_t)&dm_linux_slots[slot]);
}

static void *other_thread(void *arg) {
    const uint64_t *offsets = arg;
    for (unsigned i = 0; i < 4; i++) {
        check(i);
        assert(dm_linux_slot_offset(i) == offsets[i]);
        assert(dm_linux_slots[i] == 0);
        dm_linux_slots[i] = i + 10;
    }
    return 0;
}

int main(void) {
    uint64_t offsets[4];
    for (unsigned i = 0; i < 4; i++) {
        check(i);
        offsets[i] = dm_linux_slot_offset(i);
        dm_linux_slots[i] = i + 1;
    }
    pthread_t thread;
    assert(!pthread_create(&thread, 0, other_thread, offsets));
    assert(!pthread_join(thread, 0));
    for (unsigned i = 0; i < 4; i++) assert(dm_linux_slots[i] == i + 1);
}
