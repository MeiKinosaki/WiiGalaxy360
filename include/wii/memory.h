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


#define WII_BUS_MAX_REGIONS 8u

typedef struct {
    uint32_t base;
    size_t size;
    uint8_t *data;
    bool writable;
} wii_bus_region_t;

typedef struct {
    wii_bus_region_t regions[WII_BUS_MAX_REGIONS];
    size_t count;
} wii_bus_t;

/* Guest-address bus map. Regions must not overlap; aliases can be mapped
 * explicitly to the same backing storage at different guest addresses. */
void wii_bus_init(wii_bus_t *bus);
bool wii_bus_map(wii_bus_t *bus, uint32_t base, uint8_t *data,
                 size_t size, bool writable);
bool wii_bus_read8(const wii_bus_t *bus, uint32_t address, uint8_t *value);
bool wii_bus_read16be(const wii_bus_t *bus, uint32_t address, uint16_t *value);
bool wii_bus_read32be(const wii_bus_t *bus, uint32_t address, uint32_t *value);
bool wii_bus_write8(wii_bus_t *bus, uint32_t address, uint8_t value);
bool wii_bus_write16be(wii_bus_t *bus, uint32_t address, uint16_t value);
bool wii_bus_write32be(wii_bus_t *bus, uint32_t address, uint32_t value);

#endif
