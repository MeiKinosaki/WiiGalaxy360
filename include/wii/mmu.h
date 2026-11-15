#ifndef WII_GALAXY360_MMU_H
#define WII_GALAXY360_MMU_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "wii/memory.h"
#define WII_MMU_PAGE_SHIFT 12u
#define WII_MMU_PAGE_SIZE (1u << WII_MMU_PAGE_SHIFT)
#define WII_MMU_TLB_ENTRIES 32u
#define WII_MMU_PERM_READ  0x01u
#define WII_MMU_PERM_WRITE 0x02u
#define WII_MMU_PERM_EXEC  0x04u
#define WII_MMU_PTE1_KEY   0x00000200u
#define WII_MMU_PTE1_REFERENCED 0x00000100u
#define WII_MMU_PTE1_CHANGED   0x00000080u

typedef struct {
    uint32_t effective_page, physical_page;
    uint16_t asid;
    uint8_t permissions;
    bool valid;
} wii_mmu_tlb_entry_t;

typedef struct {
    uint32_t vsid;
    bool ks, kp, no_execute, valid;
} wii_mmu_segment_t;

typedef struct {
    uint32_t effective_base, physical_base, block_mask;
    uint8_t permissions;
    bool supervisor_valid, user_valid, valid;
} wii_mmu_bat_t;

typedef struct {
    wii_mmu_tlb_entry_t entries[WII_MMU_TLB_ENTRIES];
    size_t replacement;
    uint32_t sdr1;
    wii_mmu_segment_t segments[16];
    wii_mmu_bat_t ibat[4], dbat[4];
} wii_mmu_t;

void wii_mmu_init(wii_mmu_t *mmu);
bool wii_mmu_tlb_map(wii_mmu_t *mmu, uint32_t effective_base, uint32_t physical_base, uint16_t asid, uint8_t permissions);
bool wii_mmu_tlb_unmap(wii_mmu_t *mmu, uint32_t effective_base, uint16_t asid);
bool wii_mmu_translate(const wii_mmu_t *mmu, uint32_t effective_address, uint16_t asid, uint8_t access, uint32_t *physical_address);
bool wii_mmu_set_sdr1(wii_mmu_t *mmu, uint32_t sdr1);
bool wii_mmu_set_segment(wii_mmu_t *mmu, unsigned segment, uint32_t vsid, bool ks, bool kp, bool no_execute);
bool wii_mmu_set_segment_vsid(wii_mmu_t *mmu, unsigned segment, uint32_t vsid);
bool wii_mmu_set_bat(wii_mmu_t *mmu, bool instruction, unsigned index, uint32_t effective_base, uint32_t physical_base, uint32_t block_mask, bool supervisor_valid, bool user_valid, uint8_t permissions);
bool wii_mmu_translate_bus_ex(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t effective_address, uint16_t asid, uint8_t access, bool user_mode, uint32_t *physical_address);
bool wii_mmu_translate_bus(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t effective_address, uint16_t asid, uint8_t access, uint32_t *physical_address);
bool wii_mmu_bus_read8(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t ea, uint16_t asid, uint8_t *value);
bool wii_mmu_bus_read16be(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t ea, uint16_t asid, uint16_t *value);
bool wii_mmu_bus_write8(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t ea, uint16_t asid, uint8_t value);
bool wii_mmu_bus_write16be(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t ea, uint16_t asid, uint16_t value);
bool wii_mmu_bus_read32be(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t ea, uint16_t asid, uint32_t *value);
bool wii_mmu_bus_write32be(const wii_mmu_t *mmu, wii_bus_t *bus, uint32_t ea, uint16_t asid, uint32_t value);
#endif
