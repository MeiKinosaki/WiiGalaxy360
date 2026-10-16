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
/* Supported subset: I-form b (18); D-form cmplwi (10), cmpwi (11), addi (14), addis (15), addic (12), ori (24), xori (26); X-form
 * add (XO 266), and (XO 28), xor (XO 316), or (XO 444), subfc (XO 8).
 * b supports AA/LK; unsupported arithmetic record forms are rejected. */
ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction);
/* Memory-aware step supports lbz/lhz/lha/lwz/stb/sth/stw against bounded memory. */
ppc_step_result_t ppc_cpu_step_with_memory(ppc_cpu_t *cpu, uint32_t instruction,
                                           wii_memory_t *memory);

#endif
