#ifndef WII_GALAXY360_PPC_CPU_H
#define WII_GALAXY360_PPC_CPU_H

#include <stdint.h>
#include "ppc/ppc_alu.h"

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
/* Supported subset: D-form addi (opcode 14) and X-form subfc
 * (opcode 31 / XO 8). Unsupported instructions do not advance PC. */
ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction);

#endif
