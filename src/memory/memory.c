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
