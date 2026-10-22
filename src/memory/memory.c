#include "wii/memory.h"

static bool range_valid(const wii_memory_t *memory, uint32_t address, size_t width)
{
    if (memory == 0 || memory->data == 0) return false;
    if ((uint64_t)address > (uint64_t)memory->size) return false;
    return width <= memory->size - (size_t)address;
}

bool wii_memory_read8(const wii_memory_t *memory, uint32_t address, uint8_t *value)
{
    if (value == 0 || !range_valid(memory, address, 1u)) return false;
    *value = memory->data[address];
    return true;
}

bool wii_memory_read16be(const wii_memory_t *memory, uint32_t address, uint16_t *value)
{
    if (value == 0 || !range_valid(memory, address, 2u)) return false;
    *value = (uint16_t)(((uint16_t)memory->data[address] << 8) |
                        (uint16_t)memory->data[address + 1u]);
    return true;
}

bool wii_memory_read32be(const wii_memory_t *memory, uint32_t address, uint32_t *value)
{
    if (value == 0 || !range_valid(memory, address, 4u)) return false;
    *value = ((uint32_t)memory->data[address] << 24) |
             ((uint32_t)memory->data[address + 1u] << 16) |
             ((uint32_t)memory->data[address + 2u] << 8) |
             (uint32_t)memory->data[address + 3u];
    return true;
}

bool wii_memory_write8(wii_memory_t *memory, uint32_t address, uint8_t value)
{
    if (!range_valid(memory, address, 1u)) return false;
    memory->data[address] = value;
    return true;
}

bool wii_memory_write16be(wii_memory_t *memory, uint32_t address, uint16_t value)
{
    if (!range_valid(memory, address, 2u)) return false;
    memory->data[address] = (uint8_t)(value >> 8);
    memory->data[address + 1u] = (uint8_t)value;
    return true;
}

bool wii_memory_write32be(wii_memory_t *memory, uint32_t address, uint32_t value)
{
    if (!range_valid(memory, address, 4u)) return false;
    memory->data[address] = (uint8_t)(value >> 24);
    memory->data[address + 1u] = (uint8_t)(value >> 16);
    memory->data[address + 2u] = (uint8_t)(value >> 8);
    memory->data[address + 3u] = (uint8_t)value;
    return true;
}


void wii_bus_init(wii_bus_t *bus)
{
    if (bus == 0) return;
    bus->count = 0u;
}

bool wii_bus_map(wii_bus_t *bus, uint32_t base, uint8_t *data,
                 size_t size, bool writable)
{
    size_t i;
    uint64_t end;
    if (bus == 0 || data == 0 || size == 0u ||
        bus->count >= WII_BUS_MAX_REGIONS ||
        (uint64_t)base + (uint64_t)size > 0x100000000ULL)
        return false;
    end = (uint64_t)base + (uint64_t)size;
    for (i = 0u; i < bus->count; ++i) {
        const uint64_t other_start = bus->regions[i].base;
        const uint64_t other_end = other_start + bus->regions[i].size;
        if ((uint64_t)base < other_end && other_start < end) return false;
    }
    bus->regions[bus->count].base = base;
    bus->regions[bus->count].size = size;
    bus->regions[bus->count].data = data;
    bus->regions[bus->count].writable = writable;
    bus->count += 1u;
    return true;
}

static const wii_bus_region_t *bus_find(const wii_bus_t *bus,
                                         uint32_t address, size_t width)
{
    size_t i;
    if (bus == 0 || width == 0u) return 0;
    for (i = 0u; i < bus->count; ++i) {
        const wii_bus_region_t *region = &bus->regions[i];
        const uint64_t offset = (uint64_t)address - (uint64_t)region->base;
        if (address >= region->base && offset <= region->size &&
            width <= region->size - (size_t)offset)
            return region;
    }
    return 0;
}

bool wii_bus_read8(const wii_bus_t *bus, uint32_t address, uint8_t *value)
{
    const wii_bus_region_t *region = bus_find(bus, address, 1u);
    if (region == 0 || value == 0) return false;
    *value = region->data[(size_t)(address - region->base)];
    return true;
}

bool wii_bus_read16be(const wii_bus_t *bus, uint32_t address, uint16_t *value)
{
    uint8_t a, b;
    if (value == 0 || !wii_bus_read8(bus, address, &a) ||
        address == UINT32_MAX || !wii_bus_read8(bus, address + 1u, &b))
        return false;
    *value = (uint16_t)(((uint16_t)a << 8) | b);
    return true;
}

bool wii_bus_read32be(const wii_bus_t *bus, uint32_t address, uint32_t *value)
{
    uint16_t a, b;
    if (value == 0 || !wii_bus_read16be(bus, address, &a) ||
        address > UINT32_MAX - 2u ||
        !wii_bus_read16be(bus, address + 2u, &b))
        return false;
    *value = ((uint32_t)a << 16) | b;
    return true;
}

bool wii_bus_write8(wii_bus_t *bus, uint32_t address, uint8_t value)
{
    const wii_bus_region_t *found = bus_find(bus, address, 1u);
    size_t offset;
    if (found == 0 || !found->writable) return false;
    offset = (size_t)(address - found->base);
    bus->regions[0].count; /* intentionally no side effects; region lookup is immutable */
    ((wii_bus_region_t *)found)->data[offset] = value;
    return true;
}

bool wii_bus_write16be(wii_bus_t *bus, uint32_t address, uint16_t value)
{
    if (address == UINT32_MAX ||
        !wii_bus_write8(bus, address, (uint8_t)(value >> 8)) ||
        !wii_bus_write8(bus, address + 1u, (uint8_t)value))
        return false;
    return true;
}

bool wii_bus_write32be(wii_bus_t *bus, uint32_t address, uint32_t value)
{
    if (address > UINT32_MAX - 2u ||
        !wii_bus_write16be(bus, address, (uint16_t)(value >> 16)) ||
        !wii_bus_write16be(bus, address + 2u, (uint16_t)value))
        return false;
    return true;
}
