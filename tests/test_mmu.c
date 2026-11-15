#include <assert.h>
#include <stdint.h>
#include "wii/mmu.h"

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
    return 0;
}
