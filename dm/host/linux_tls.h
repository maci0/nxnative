#ifndef DM_LINUX_TLS_H
#define DM_LINUX_TLS_H
#include <stdint.h>

/* K18, KTLS, KTLR, KGATE: offsets from Linux's TPIDR_EL0, not pthread key numbers. */
extern __thread uint64_t dm_linux_slots[4];
uint64_t dm_linux_slot_offset(unsigned slot);

#endif
