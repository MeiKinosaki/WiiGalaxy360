#ifndef WII_GALAXY360_MEMORY_H
#define WII_GALAXY360_MEMORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t size;
} wii_memory_t;

/* Byte-addressed memory helpers. Multi-byte values use PowerPC big-endian order.
 * Failed accesses (null memory, null backing store, or out-of-range address)
 * return false and do not modify memory or the output value. */
bool wii_memory_read8(const wii_memory_t *memory, uint32_t address, uint8_t *value);
bool wii_memory_read16be(const wii_memory_t *memory, uint32_t address, uint16_t *value);
bool wii_memory_read32be(const wii_memory_t *memory, uint32_t address, uint32_t *value);
bool wii_memory_write8(wii_memory_t *memory, uint32_t address, uint8_t value);
bool wii_memory_write16be(wii_memory_t *memory, uint32_t address, uint16_t value);
bool wii_memory_write32be(wii_memory_t *memory, uint32_t address, uint32_t value);

#endif
