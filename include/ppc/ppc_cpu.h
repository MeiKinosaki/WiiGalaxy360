#ifndef WII_GALAXY360_PPC_CPU_H
#define WII_GALAXY360_PPC_CPU_H

#include <stdint.h>
#include "ppc/ppc_alu.h"
#include "wii/memory.h"

typedef struct {
    uint32_t gpr[32];
    uint32_t pc;
    uint32_t lr;
    uint32_t ctr;
    uint32_t cr;
    ppc_alu_state_t alu;
} ppc_cpu_t;

typedef enum {
    PPC_STEP_OK = 0,
    PPC_STEP_UNSUPPORTED = 1,
    PPC_STEP_ALIGNMENT = 2
} ppc_step_result_t;

void ppc_cpu_reset(ppc_cpu_t *cpu, uint32_t entry_pc);
/* Supported subset: B-form bc (16), I-form b (18); D-form mulli (7), subfic (8), cmplwi (10), cmpwi (11), addi (14), addis (15), addic (12), ori (24), xori (26), M-form rlwinm (21); X-form mfspr (XO 339) and mtspr (XO 467) for LR/CTR; nor (XO 124), andc (XO 60), cntlzw (XO 26), extsb (XO 954), extsh (XO 922), subfze (XO 200); addc (XO 10), addze (XO 202), subf (XO 40), neg (XO 104), mullw (XO 235), add (XO 266), and (XO 28), xor (XO 316), or (XO 444), subfc (XO 8).
 * b/bc support AA/LK; bc evaluates CR and CTR conditions; unsupported arithmetic record forms are rejected. */
ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction);
/* Fetch one big-endian instruction from memory at PC, then execute it. */
ppc_step_result_t ppc_cpu_step_from_bus(ppc_cpu_t *cpu, wii_bus_t *bus);
ppc_step_result_t ppc_cpu_step_from_memory(ppc_cpu_t *cpu, wii_memory_t *memory);
/* Memory-aware step supports lbz/lhz/lha/lwz/stb/sth/stw against bounded memory. */
ppc_step_result_t ppc_cpu_step_with_memory(ppc_cpu_t *cpu, uint32_t instruction,
                                           wii_memory_t *memory);
/* Memory-aware step using mapped guest addresses for both fetch and data. */
ppc_step_result_t ppc_cpu_step_with_bus(ppc_cpu_t *cpu, uint32_t instruction,
                                        wii_bus_t *bus);

#endif
