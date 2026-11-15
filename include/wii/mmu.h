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

typedef struct {
    uint32_t effective_page;
    uint32_t physical_page;
    uint16_t asid;
    uint8_t permissions;
    bool valid;
} wii_mmu_tlb_entry_t;

typedef struct {
    wii_mmu_tlb_entry_t entries[WII_MMU_TLB_ENTRIES];
    size_t replacement;
    uint32_t sdr1;
    uint32_t segment_vsid[16];
    bool segment_valid[16];
} wii_mmu_t;

/* A deterministic software TLB scaffold. This is not yet the Broadway
 * hashed page-table walker: SDR1/PTEG/PTE matching and segment registers
 * must be implemented before claiming hardware-faithful MMU support. */
void wii_mmu_init(wii_mmu_t *mmu);
bool wii_mmu_tlb_map(wii_mmu_t *mmu, uint32_t effective_base,
                     uint32_t physical_base, uint16_t asid,
                     uint8_t permissions);
bool wii_mmu_tlb_unmap(wii_mmu_t *mmu, uint32_t effective_base,
                       uint16_t asid);
bool wii_mmu_translate(const wii_mmu_t *mmu, uint32_t effective_address,
                       uint16_t asid, uint8_t access,
                       uint32_t *physical_address);
/* Configure the PowerPC hashed page table and one of 16 segment VSIDs.
 * SDR1 is retained in its architectural register encoding. */
void wii_mmu_set_sdr1(wii_mmu_t *mmu, uint32_t sdr1);
bool wii_mmu_set_segment_vsid(wii_mmu_t *mmu, unsigned segment,
                              uint32_t vsid);
/* TLB-first translation; on miss, walk primary then secondary PTEGs. */
bool wii_mmu_translate_bus(const wii_mmu_t *mmu, const wii_bus_t *bus,
                           uint32_t effective_address, uint16_t asid,
                           uint8_t access, uint32_t *physical_address);
/* Translate an effective address through the software TLB, then access
 * the physical bus. These helpers do not implement page-table walks. */
bool wii_mmu_bus_read8(const wii_mmu_t *mmu, const wii_bus_t *bus,
                       uint32_t effective_address, uint16_t asid,
                       uint8_t *value);
bool wii_mmu_bus_read16be(const wii_mmu_t *mmu, const wii_bus_t *bus,
                          uint32_t effective_address, uint16_t asid,
                          uint16_t *value);
bool wii_mmu_bus_write8(const wii_mmu_t *mmu, wii_bus_t *bus,
                        uint32_t effective_address, uint16_t asid,
                        uint8_t value);
bool wii_mmu_bus_write16be(const wii_mmu_t *mmu, wii_bus_t *bus,
                           uint32_t effective_address, uint16_t asid,
                           uint16_t value);

bool wii_mmu_bus_read32be(const wii_mmu_t *mmu, const wii_bus_t *bus,
                          uint32_t effective_address, uint16_t asid,
                          uint32_t *value);
bool wii_mmu_bus_write32be(const wii_mmu_t *mmu, wii_bus_t *bus,
                           uint32_t effective_address, uint16_t asid,
                           uint32_t value);

#endif
