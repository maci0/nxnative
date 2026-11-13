#define _GNU_SOURCE
#include "linux_guest_alias.h"
#ifdef __linux__
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

enum { GUEST_BITS = 39 };
static const uintptr_t guest_size = 1ull << GUEST_BITS;
static const uintptr_t high_base = 0x100000000000ull;
static const uintptr_t module_base = 0x800000000ull;
// hle_svc reserves 256 MB before the heap; Hookshot's GPU page table starts at 64 GB.
static const uintptr_t as_base = 0xff0000000ull;
static uintptr_t next_module = module_base;
static int fd = -1, ready;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
extern int hos_posix_guest_memfd(void) __attribute__((weak));

int dm_linux_guest_alias_enabled(void) {
    const char *s = getenv("DM_NATIVE_GUEST_ALIAS");
    return s && *s && atoi(s) != 0;
}

static void init(void) {
    if (ready) return;
    if (hos_posix_guest_memfd && (fd = hos_posix_guest_memfd()) >= 0) {
        struct stat st;
        unsigned char vec;
        if (fstat(fd, &st) || st.st_size != (off_t)guest_size ||
            mincore((void *)module_base, 0x1000, &vec) ||
            mincore((void *)(high_base + module_base), 0x1000, &vec)) abort();
        ready = 1;
        return;
    }
    fd = (int)syscall(SYS_memfd_create, "nxnative-guest", 3u);
    if (fd < 0 || ftruncate(fd, (off_t)guest_size)) abort();
    void *high = mmap((void *)high_base, guest_size, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_FIXED_NOREPLACE | MAP_NORESERVE, fd, 0);
    if (high != (void *)high_base) abort();
    void *low = mmap((void *)(1ull << 20), guest_size - (1ull << 20), PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_FIXED_NOREPLACE | MAP_NORESERVE, fd, 1ull << 20);
    if (low != (void *)(1ull << 20)) abort();
    ready = 1;
}

void *dm_linux_guest_alias_alloc(size_t size) {
    pthread_mutex_lock(&lock);
    init();
    size = (size + 0xffff) & ~(size_t)0xffff;
    uintptr_t p = next_module;
    if (size > as_base - p) abort();
    next_module += size;
    pthread_mutex_unlock(&lock);
    return (void *)p;
}

void *dm_linux_guest_alias_as(size_t size) {
    pthread_mutex_lock(&lock);
    init();
    if (size > guest_size - as_base || mprotect((void *)as_base, size, PROT_NONE)) abort();
    pthread_mutex_unlock(&lock);
    return (void *)as_base;
}

int dm_linux_guest_alias_fd(void) {
    pthread_mutex_lock(&lock);
    init();
    int result = fd;
    pthread_mutex_unlock(&lock);
    return result;
}
#endif
