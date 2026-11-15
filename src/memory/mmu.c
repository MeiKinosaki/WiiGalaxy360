#include "wii/mmu.h"

void wii_mmu_init(wii_mmu_t *mmu)
{
    size_t i;
    if (mmu == 0) return;
    for (i = 0u; i < WII_MMU_TLB_ENTRIES; ++i)
        mmu->entries[i].valid = false;
    mmu->replacement = 0u;
    mmu->sdr1 = 0u;
    {
        size_t segment;
        for (segment = 0u; segment < 16u; ++segment) {
            mmu->segment_vsid[segment] = 0u;
            mmu->segment_valid[segment] = false;
        }
    }
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


void wii_mmu_set_sdr1(wii_mmu_t *mmu, uint32_t sdr1)
{
    if (mmu != 0) mmu->sdr1 = sdr1;
}

bool wii_mmu_set_segment_vsid(wii_mmu_t *mmu, unsigned segment,
                              uint32_t vsid)
{
    if (mmu == 0 || segment >= 16u || vsid > 0x00ffffffu) return false;
    mmu->segment_vsid[segment] = vsid;
    mmu->segment_valid[segment] = true;
    return true;
}

static bool wii_mmu_walk_pteg(const wii_bus_t *bus,
                              uint32_t pteg_address, uint32_t vsid,
                              uint32_t api, bool secondary, uint32_t offset,
                              uint8_t access, uint32_t *physical_address)
{
    unsigned slot;
    for (slot = 0u; slot < 8u; ++slot) {
        uint32_t pte0, pte1;
        const uint32_t address = pteg_address + slot * 8u;
        if (!wii_bus_read32be(bus, address, &pte0) ||
            !wii_bus_read32be(bus, address + 4u, &pte1))
            return false;
        if ((pte0 & 0x80000000u) == 0u ||
            ((pte0 >> 7) & 0x00ffffffu) != vsid ||
            (((pte0 >> 6) & 1u) != 0u) != secondary ||
            (pte0 & 0x3fu) != api)
            continue;
        {
            const uint32_t pp = pte1 & 3u;
            const bool readable = pp != 0u;
            const bool writable = pp == 1u;
            if ((access == WII_MMU_PERM_WRITE && !writable) ||
                ((access == WII_MMU_PERM_READ ||
                  access == WII_MMU_PERM_EXEC) && !readable))
                return false;
        }
        *physical_address = (pte1 & 0xfffff000u) | offset;
        return true;
    }
    return false;
}

bool wii_mmu_translate_bus(const wii_mmu_t *mmu, const wii_bus_t *bus,
                           uint32_t effective_address, uint16_t asid,
                           uint8_t access, uint32_t *physical_address)
{
    uint32_t vsid, page_index, api, hash, mask, origin, pteg;
    const unsigned segment = effective_address >> 28;
    const uint32_t offset = effective_address & 0xfffu;
    if (mmu == 0 || bus == 0 || physical_address == 0 ||
        (access != WII_MMU_PERM_READ && access != WII_MMU_PERM_WRITE &&
         access != WII_MMU_PERM_EXEC))
        return false;
    if (wii_mmu_translate(mmu, effective_address, asid, access,
                          physical_address))
        return true;
    if (!mmu->segment_valid[segment]) return false;
    vsid = mmu->segment_vsid[segment];
    page_index = (effective_address >> 12) & 0xffffu;
    api = (effective_address >> 22) & 0x3fu;
    hash = vsid ^ page_index;
    mask = mmu->sdr1 & 0x1ffu;
    origin = mmu->sdr1 & 0xffff0000u;
    pteg = origin | ((hash & mask) << 6);
    if (wii_mmu_walk_pteg(bus, pteg, vsid, api, false, offset,
                          access, physical_address))
        return true;
    pteg = origin | (((~hash) & mask) << 6);
    return wii_mmu_walk_pteg(mmu, bus, pteg, vsid, api, true, offset,
                             access, physical_address);
}

bool wii_mmu_bus_read32be(const wii_mmu_t *mmu, const wii_bus_t *bus,
                          uint32_t effective_address, uint16_t asid,
                          uint32_t *value)
{
    uint32_t physical_address;
    if (value == 0 ||
        !wii_mmu_translate_bus(mmu, bus, effective_address, asid,
                           WII_MMU_PERM_READ, &physical_address))
        return false;
    return wii_bus_read32be(bus, physical_address, value);
}

bool wii_mmu_bus_write32be(const wii_mmu_t *mmu, wii_bus_t *bus,
                           uint32_t effective_address, uint16_t asid,
                           uint32_t value)
{
    uint32_t physical_address;
    if (!wii_mmu_translate_bus(mmu, bus, effective_address, asid,
                           WII_MMU_PERM_WRITE, &physical_address))
        return false;
    return wii_bus_write32be(bus, physical_address, value);
}

bool wii_mmu_bus_read8(const wii_mmu_t *mmu, const wii_bus_t *bus,
                       uint32_t effective_address, uint16_t asid,
                       uint8_t *value)
{
    uint32_t physical_address;
    if (value == 0 ||
        !wii_mmu_translate_bus(mmu, bus, effective_address, asid,
                           WII_MMU_PERM_READ, &physical_address))
        return false;
    return wii_bus_read8(bus, physical_address, value);
}

bool wii_mmu_bus_read16be(const wii_mmu_t *mmu, const wii_bus_t *bus,
                          uint32_t effective_address, uint16_t asid,
                          uint16_t *value)
{
    uint32_t physical_address;
    if (value == 0 ||
        !wii_mmu_translate_bus(mmu, bus, effective_address, asid,
                           WII_MMU_PERM_READ, &physical_address))
        return false;
    return wii_bus_read16be(bus, physical_address, value);
}

bool wii_mmu_bus_write8(const wii_mmu_t *mmu, wii_bus_t *bus,
                        uint32_t effective_address, uint16_t asid,
                        uint8_t value)
{
    uint32_t physical_address;
    if (!wii_mmu_translate_bus(mmu, bus, effective_address, asid,
                           WII_MMU_PERM_WRITE, &physical_address))
        return false;
    return wii_bus_write8(bus, physical_address, value);
}

bool wii_mmu_bus_write16be(const wii_mmu_t *mmu, wii_bus_t *bus,
                           uint32_t effective_address, uint16_t asid,
                           uint16_t value)
{
    uint32_t physical_address;
    if (!wii_mmu_translate_bus(mmu, bus, effective_address, asid,
                           WII_MMU_PERM_WRITE, &physical_address))
        return false;
    return wii_bus_write16be(bus, physical_address, value);
}
