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
    return 0;
}
