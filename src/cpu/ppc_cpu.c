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

    /* D-form addi: RT <- (RA == 0 ? 0 : GPR[RA]) + sign_extend(SI). */
    if (opcode == 14u) {
        const uint32_t raw_imm = instruction & 0xffffu;
        const int32_t simm = (raw_imm & 0x8000u)
            ? (int32_t)raw_imm - 65536
            : (int32_t)raw_imm;
        const uint32_t base = (ra == 0u) ? 0u : cpu->gpr[ra];
        cpu->gpr[rt] = base + (uint32_t)simm;
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    /* D-form addic: RT <- RA + sign_extend(SI), updating XER[CA]. */
    if (opcode == 12u) {
        const uint32_t raw_imm = instruction & 0xffffu;
        const int32_t simm = (raw_imm & 0x8000u)
            ? (int32_t)raw_imm - 65536
            : (int32_t)raw_imm;
        const uint32_t base = (ra == 0u) ? 0u : cpu->gpr[ra];
        const uint64_t wide_sum = (uint64_t)base + (uint64_t)(uint32_t)simm;
        cpu->gpr[rt] = (uint32_t)wide_sum;
        cpu->alu.xer_ca = (uint32_t)(wide_sum >> 32);
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    /* D-form ori: RA <- RS | zero_extend(UI); unlike addi, RA=0 is a GPR. */
    if (opcode == 24u) {
        const uint32_t uimm = instruction & 0xffffu;
        cpu->gpr[ra] = cpu->gpr[rt] | uimm;
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    if (opcode == 31u) {
        /* X-form add: RT <- RA + RB; does not update XER[CA]. */
        if (xo == 266u) {
            cpu->gpr[rt] = cpu->gpr[ra] + cpu->gpr[rb];
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form logical instructions use RS (encoded in rt) and RB,
         * writing the result to RA. Rc=0 is required by this subset. */
        if ((instruction & 1u) == 0u) {
            if (xo == 28u) {
                cpu->gpr[ra] = ppc_and(cpu->gpr[rt], cpu->gpr[rb]);
                cpu->pc += 4u;
                return PPC_STEP_OK;
            }
            if (xo == 444u) {
                cpu->gpr[ra] = ppc_or(cpu->gpr[rt], cpu->gpr[rb]);
                cpu->pc += 4u;
                return PPC_STEP_OK;
            }
            if (xo == 316u) {
                cpu->gpr[ra] = ppc_xor(cpu->gpr[rt], cpu->gpr[rb]);
                cpu->pc += 4u;
                return PPC_STEP_OK;
            }
        }

        /* X-form subfc: RT <- RB - RA, XER[CA]=1 when no borrow. */
        if (xo == 8u) {
            cpu->gpr[rt] = ppc_subfc(&cpu->alu, cpu->gpr[ra], cpu->gpr[rb]);
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }
    }

    return PPC_STEP_UNSUPPORTED;
}
