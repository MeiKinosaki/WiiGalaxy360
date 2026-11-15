#ifndef WII_GALAXY360_MMU_H
#define WII_GALAXY360_MMU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

#endif
