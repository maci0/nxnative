#define _GNU_SOURCE
#include "../linux_guest_alias.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main(void) {
    assert(!setenv("DM_NATIVE_GUEST_ALIAS", "1", 1));
    assert(dm_linux_guest_alias_enabled());
    volatile uint8_t *image = dm_linux_guest_alias_alloc(0x10000);
    assert((uintptr_t)image == 0x800000000ull);
    volatile uint8_t *high = (volatile uint8_t *)((uintptr_t)image + 0x100000000000ull);
    image[7] = 0x5a;
    assert(high[7] == 0x5a);
    high[8] = 0xa5;
    assert(image[8] == 0xa5);
    volatile uint8_t *as = dm_linux_guest_alias_as(0x10000000);
    assert((uintptr_t)as + 0x10000000ull == 0x1000000000ull);
    assert((uintptr_t)image + 0x10000 <= (uintptr_t)as);
    assert(dm_linux_guest_alias_fd() >= 0);
    assert(!mprotect((void *)as, 0x1000, PROT_READ | PROT_WRITE));
    as[0] = 0x3c;
    assert(as[0x100000000000ull] == 0x3c);
    assert(!mprotect((void *)as, 0x1000, PROT_NONE));
    assert(!madvise((void *)as, 0x1000, MADV_REMOVE));
    assert(as[0x100000000000ull] == 0);
    puts("LINUX_GUEST_ALIAS_PASS");
}
