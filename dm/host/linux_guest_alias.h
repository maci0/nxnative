#pragma once
#include <stddef.h>

#ifdef __linux__
int dm_linux_guest_alias_enabled(void);
void *dm_linux_guest_alias_alloc(size_t size);
void *dm_linux_guest_alias_as(size_t size);
int dm_linux_guest_alias_fd(void);
#endif
