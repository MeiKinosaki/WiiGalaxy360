#include <assert.h>
#include <stdint.h>
#include "wii/mmu.h"

static void store_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}


int main(void)
{
    wii_mmu_t mmu;
    uint32_t physical = 0xdeadbeefu;
    wii_mmu_init(&mmu);
    assert(!wii_mmu_translate(&mmu, 0x1234u, 1u, WII_MMU_PERM_READ, &physical));
    assert(wii_mmu_tlb_map(&mmu, 0x1000u, 0x8000u, 7u,
                           WII_MMU_PERM_READ | WII_MMU_PERM_WRITE));
    assert(wii_mmu_translate(&mmu, 0x1abcu, 7u, WII_MMU_PERM_READ, &physical));
    assert(physical == 0x8abcu);
    assert(wii_mmu_translate(&mmu, 0x1abcu, 7u, WII_MMU_PERM_WRITE, &physical));
    assert(!wii_mmu_translate(&mmu, 0x1abcu, 7u, WII_MMU_PERM_EXEC, &physical));
    assert(!wii_mmu_translate(&mmu, 0x1abcu, 8u, WII_MMU_PERM_READ, &physical));
    assert(wii_mmu_tlb_map(&mmu, 0x1000u, 0x9000u, 7u, WII_MMU_PERM_EXEC));
    assert(wii_mmu_translate(&mmu, 0x1004u, 7u, WII_MMU_PERM_EXEC, &physical));
    assert(physical == 0x9004u);
    assert(!wii_mmu_tlb_map(&mmu, 0x1001u, 0x9000u, 7u, WII_MMU_PERM_READ));
    assert(!wii_mmu_tlb_map(&mmu, 0x2000u, 0x9000u, 7u, 0u));
    assert(wii_mmu_tlb_unmap(&mmu, 0x1000u, 7u));
    assert(!wii_mmu_translate(&mmu, 0x1004u, 7u, WII_MMU_PERM_EXEC, &physical));
    assert(!wii_mmu_tlb_unmap(&mmu, 0x1000u, 7u));
    assert(!wii_mmu_translate(&mmu, 0x1000u, 7u, WII_MMU_PERM_READ, 0));

    /* Fill all entries, then verify deterministic round-robin replacement. */
    wii_mmu_init(&mmu);
    {
        unsigned i;
        for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i) {
            assert(wii_mmu_tlb_map(&mmu, 0x10000u + i * WII_MMU_PAGE_SIZE,
                                   0x80000u + i * WII_MMU_PAGE_SIZE, 3u,
                                   WII_MMU_PERM_READ));
        }
        for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i) {
            assert(wii_mmu_translate(&mmu, 0x10000u + i * WII_MMU_PAGE_SIZE + 4u,
                                     3u, WII_MMU_PERM_READ, &physical));
            assert(physical == 0x80000u + i * WII_MMU_PAGE_SIZE + 4u);
        }
        assert(wii_mmu_tlb_map(&mmu, 0x50000u, 0x90000u, 3u,
                               WII_MMU_PERM_READ));
        assert(!wii_mmu_translate(&mmu, 0x10004u, 3u,
                                  WII_MMU_PERM_READ, &physical));
        assert(wii_mmu_translate(&mmu, 0x50004u, 3u,
                                 WII_MMU_PERM_READ, &physical));
        assert(physical == 0x90004u);
        assert(wii_mmu_translate(&mmu, 0x11004u, 3u,
                                 WII_MMU_PERM_READ, &physical));
    }

    /* Exercise virtual-to-physical translation against Wii RAM aliases. */
    {
        uint8_t mem1[0x2000] = {0};
        uint8_t mem2[0x1000] = {0};
        wii_bus_t bus;
        wii_mmu_init(&mmu);
        wii_bus_init(&bus);
        assert(wii_bus_map_wii_ram(&bus, mem1, sizeof(mem1), mem2, sizeof(mem2)));
        assert(wii_mmu_tlb_map(&mmu, 0x40000000u, 0x80000000u, 9u,
                               WII_MMU_PERM_READ | WII_MMU_PERM_WRITE));
        assert(wii_mmu_bus_write32be(&mmu, &bus, 0x40000020u, 9u, 0xcafebabeu));
        assert(wii_mmu_bus_read32be(&mmu, &bus, 0x40000020u, 9u, &physical));
        assert(physical == 0xcafebabeu);
        assert(!wii_mmu_bus_write32be(&mmu, &bus, 0x40000020u, 10u, 0u));
        assert(wii_mmu_tlb_map(&mmu, 0x40000000u, 0x80000000u, 9u,
                               WII_MMU_PERM_READ));
        assert(!wii_mmu_bus_write32be(&mmu, &bus, 0x40000020u, 9u, 0u));
    }

    /* Hashed page table: SDR1 -> primary PTEG, VSID/API tag, RPN and PP. */
    {
        uint8_t mem1[0x3000] = {0};
        uint8_t mem2[0x1000] = {0};
        wii_bus_t bus;
        uint32_t translated = 0u;
        const uint32_t vsid = 0x12345u;
        const uint32_t pte0 = 0x80000000u | (vsid << 7);
        const uint32_t pte1 = 0x80001000u | 1u; /* PP=01: read/write */
        wii_mmu_init(&mmu);
        wii_bus_init(&bus);
        assert(wii_bus_map_wii_ram(&bus, mem1, sizeof(mem1), mem2, sizeof(mem2)));
        wii_mmu_set_sdr1(&mmu, 0x80000000u); /* HTABORG=0x80000000, mask=0 */
        assert(wii_mmu_set_segment_vsid(&mmu, 0u, vsid));
        store_be32(&mem1[0], pte0);
        store_be32(&mem1[4], pte1);
        mem1[0x1234u] = 0x5au;
        assert(wii_mmu_translate_bus(&mmu, &bus, 0x00000234u, 0u,
                                     WII_MMU_PERM_READ, &translated));
        assert(translated == 0x80001234u);
        assert(wii_mmu_bus_read8(&mmu, &bus, 0x00000234u, 0u, &mem2[0]));
        assert(mem2[0] == 0x5au);
        assert(wii_mmu_bus_write8(&mmu, &bus, 0x00000235u, 0u, 0xa5u));
        assert(mem1[0x1235u] == 0xa5u);
        assert(!wii_mmu_translate_bus(&mmu, &bus, 0x00000234u, 0u,
                                      WII_MMU_PERM_EXEC, &translated) == 0);
    }
    return 0;
}
