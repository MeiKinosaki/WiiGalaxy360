#ifndef WII_GALAXY360_PPC_CPU_H
#define WII_GALAXY360_PPC_CPU_H

#include <stdint.h>
#include "ppc/ppc_alu.h"
#include "wii/memory.h"
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
    PPC_STEP_UNSUPPORTED = 1
} ppc_step_result_t;

void ppc_cpu_reset(ppc_cpu_t *cpu, uint32_t entry_pc);
/* Supported subset: D-form addi (14), addic (12), ori (24), and X-form
 * add (XO 266), and (XO 28), xor (XO 316), or (XO 444), subfc (XO 8).
 * Logical Rc=1 variants and unsupported instructions are rejected. */
ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction);
/* Memory-aware step supports lwz/lbz/stw/stb against a bounded memory region. */
ppc_step_result_t ppc_cpu_step_with_memory(ppc_cpu_t *cpu, uint32_t instruction,
                                           wii_memory_t *memory);
/* Memory-aware step supports lwz/lbz/stw/stb against a bounded memory region. */
ppc_step_result_t ppc_cpu_step_with_memory(ppc_cpu_t *cpu, uint32_t instruction,
                                           wii_memory_t *memory);

#endif
