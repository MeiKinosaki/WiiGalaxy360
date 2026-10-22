#include <assert.h>
#include <stdint.h>
#include "wii/memory.h"

int main(void)
{
    uint8_t bytes[8] = {0};
    wii_memory_t memory = { bytes, sizeof(bytes) };
    uint8_t b = 0u;
    uint16_t h = 0u;
    uint32_t w = 0u;

    assert(wii_memory_write8(&memory, 0u, 0xa5u));
    assert(wii_memory_read8(&memory, 0u, &b) && b == 0xa5u);

    assert(wii_memory_write16be(&memory, 1u, 0x1234u));
    assert(bytes[1] == 0x12u && bytes[2] == 0x34u);
    assert(wii_memory_read16be(&memory, 1u, &h) && h == 0x1234u);

    assert(wii_memory_write32be(&memory, 3u, 0x89abcdefu));
    assert(bytes[3] == 0x89u && bytes[4] == 0xabu &&
           bytes[5] == 0xcdu && bytes[6] == 0xefu);
    assert(wii_memory_read32be(&memory, 3u, &w) && w == 0x89abcdefu);

    /* Out-of-range operations fail and leave output values unchanged. */
    b = 0x5au;
    assert(!wii_memory_read8(&memory, 8u, &b) && b == 0x5au);
    assert(!wii_memory_read16be(&memory, 7u, &h) && h == 0x1234u);
    assert(!wii_memory_read32be(&memory, 5u, &w) && w == 0x89abcdefu);
    assert(!wii_memory_write32be(&memory, 6u, 0u));
    assert(!wii_memory_read8(0, 0u, &b));
    assert(!wii_memory_write8(0, 0u, 1u));
    assert(!wii_memory_read8(&memory, 0u, 0));

    /* Guest bus: mapped RAM, read-only regions, aliases and unmapped faults. */
    {
        uint8_t ram[16] = {0};
        uint8_t rom[4] = {0x12u, 0x34u, 0x56u, 0x78u};
        wii_bus_t bus;
        uint32_t word = 0u;
        uint8_t byte = 0u;
        wii_bus_init(&bus);
        assert(wii_bus_map(&bus, 0x1000u, ram, sizeof(ram), true));
        assert(wii_bus_map(&bus, 0x8000u, ram, sizeof(ram), true));
        assert(wii_bus_map(&bus, 0xfff00000u, rom, sizeof(rom), false));
        assert(!wii_bus_map(&bus, 0x1008u, rom, sizeof(rom), true));
        assert(wii_bus_write32be(&bus, 0x1004u, 0x89abcdefu));
        assert(wii_bus_read32be(&bus, 0x8004u, &word) && word == 0x89abcdefu);
        assert(wii_bus_read32be(&bus, 0xfff00000u, &word) && word == 0x12345678u);
        assert(!wii_bus_write8(&bus, 0xfff00000u, 0u));
        assert(!wii_bus_read8(&bus, 0x4000u, &byte));
        assert(!wii_bus_read32be(&bus, 0x100eu, &word));
        assert(!wii_bus_map(&bus, 0xffffffffu, rom, 4u, true));
    }


    /* Wii Broadway RAM aliases share backing storage at cached/uncached and
     * physical views. Small backing arrays keep the host test lightweight. */
    {
        uint8_t mem1[64] = {0};
        uint8_t mem2[32] = {0};
        wii_bus_t bus;
        uint32_t word = 0u;
        wii_bus_init(&bus);
        assert(wii_bus_map_wii_ram(&bus, mem1, sizeof(mem1), mem2, sizeof(mem2)));
        assert(bus.count == 6u);
        assert(wii_bus_write32be(&bus, 0x80000010u, 0xdecafbadU));
        assert(wii_bus_read32be(&bus, 0x00000010u, &word) && word == 0xdecafbadU);
        assert(wii_bus_read32be(&bus, 0xc0000010u, &word) && word == 0xdecafbadU);
        assert(wii_bus_write32be(&bus, 0x90000004u, 0x12345678u));
        assert(wii_bus_read32be(&bus, 0x10000004u, &word) && word == 0x12345678u);
        assert(wii_bus_read32be(&bus, 0xd0000004u, &word) && word == 0x12345678u);
        assert(!wii_bus_map_wii_ram(&bus, mem1, sizeof(mem1), mem2, sizeof(mem2)));
    }

    return 0;
}
