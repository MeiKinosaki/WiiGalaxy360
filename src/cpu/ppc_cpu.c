#include "ppc/ppc_cpu.h"

void ppc_cpu_reset(ppc_cpu_t *cpu, uint32_t entry_pc)
{
    unsigned i;
    if (cpu == 0) return;
    for (i = 0; i < 32u; ++i) cpu->gpr[i] = 0u;
    cpu->pc = entry_pc;
    cpu->lr = 0u;
    cpu->ctr = 0u;
    cpu->cr = 0u;
    cpu->alu.xer_ca = 0u;
}

ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction)
{
    const uint32_t opcode = instruction >> 26;
    const uint32_t rt = (instruction >> 21) & 31u;
    const uint32_t ra = (instruction >> 16) & 31u;
    const uint32_t rb = (instruction >> 11) & 31u;
    const uint32_t xo = (instruction >> 1) & 1023u;

    if (cpu == 0) return PPC_STEP_UNSUPPORTED;
    if (opcode != 31u || xo != 8u) return PPC_STEP_UNSUPPORTED;

    cpu->gpr[rt] = ppc_subfc(&cpu->alu, cpu->gpr[ra], cpu->gpr[rb]);
    cpu->pc += 4u;
    return PPC_STEP_OK;
}
