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
    return 0;
}
