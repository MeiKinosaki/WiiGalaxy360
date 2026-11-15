#include "wii/mmu.h"

void wii_mmu_init(wii_mmu_t *mmu)
{
    size_t i;
    if (mmu == 0) return;
    for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i)
        mmu->entries[i].valid = false;
    mmu->replacement = 0u;
}

bool wii_mmu_tlb_map(wii_mmu_t *mmu, uint32_t effective_base,
                     uint32_t physical_base, uint16_t asid,
                     uint8_t permissions)
{
    size_t i, slot = WII_MMU_TLB_ENTRIES;
    if (mmu == 0 || (effective_base & (WII_MMU_PAGE_SIZE - 1u)) != 0u ||
        (physical_base & (WII_MMU_PAGE_SIZE - 1u)) != 0u ||
        permissions == 0u ||
        (permissions & ~(WII_MMU_PERM_READ | WII_MMU_PERM_WRITE |
                         WII_MMU_PERM_EXEC)) != 0u)
        return false;

    for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i) {
        wii_mmu_tlb_entry_t *entry = &mmu->entries[i];
        if (entry->valid && entry->effective_page == effective_base &&
            entry->asid == asid) {
            slot = i;
            break;
        }
        if (!entry->valid && slot == WII_MMU_TLB_ENTRIES) slot = i;
    }
    if (slot == WII_MMU_TLB_ENTRIES) {
        slot = mmu->replacement;
        mmu->replacement = (mmu->replacement + 1u) % WII_MMU_TLB_ENTRIES;
    }
    mmu->entries[slot].effective_page = effective_base;
    mmu->entries[slot].physical_page = physical_base;
    mmu->entries[slot].asid = asid;
    mmu->entries[slot].permissions = permissions;
    mmu->entries[slot].valid = true;
    return true;
}

bool wii_mmu_tlb_unmap(wii_mmu_t *mmu, uint32_t effective_base,
                       uint16_t asid)
{
    size_t i;
    if (mmu == 0 || (effective_base & (WII_MMU_PAGE_SIZE - 1u)) != 0u)
        return false;
    for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i) {
        wii_mmu_tlb_entry_t *entry = &mmu->entries[i];
        if (entry->valid && entry->effective_page == effective_base &&
            entry->asid == asid) {
            entry->valid = false;
            return true;
        }
    }
    return false;
}

bool wii_mmu_translate(const wii_mmu_t *mmu, uint32_t effective_address,
                       uint16_t asid, uint8_t access,
                       uint32_t *physical_address)
{
    size_t i;
    const uint32_t page = effective_address & ~(WII_MMU_PAGE_SIZE - 1u);
    const uint32_t offset = effective_address & (WII_MMU_PAGE_SIZE - 1u);
    if (mmu == 0 || physical_address == 0 ||
        (access != WII_MMU_PERM_READ && access != WII_MMU_PERM_WRITE &&
         access != WII_MMU_PERM_EXEC))
        return false;
    for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i) {
        const wii_mmu_tlb_entry_t *entry = &mmu->entries[i];
        if (entry->valid && entry->effective_page == page &&
            entry->asid == asid) {
            if ((entry->permissions & access) == 0u ||
                entry->physical_page > UINT32_MAX - offset)
                return false;
            *physical_address = entry->physical_page + offset;
            return true;
        }
    }
    return false;
}
