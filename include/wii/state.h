#ifndef WII_GALAXY360_STATE_H
#define WII_GALAXY360_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ppc/ppc_cpu.h"

/* Versioned host-side save state for the currently implemented CPU subset
 * and RAM. This does not yet capture GPU/audio/IOS/FPU/MMU/device state. */
bool wii_state_save(const char *path, const ppc_cpu_t *cpu,
                    const uint8_t *mem1, size_t mem1_size,
                    const uint8_t *mem2, size_t mem2_size);
bool wii_state_load(const char *path, ppc_cpu_t *cpu,
                    uint8_t *mem1, size_t mem1_size,
                    uint8_t *mem2, size_t mem2_size);

#endif
