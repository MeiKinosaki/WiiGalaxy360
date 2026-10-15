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

ppc_step_result_t ppc_cpu_step_with_memory(ppc_cpu_t *cpu, uint32_t instruction, wii_memory_t *memory)
{
    const uint32_t opcode = instruction >> 26;
    const uint32_t rt = (instruction >> 21) & 31u;
    const uint32_t ra = (instruction >> 16) & 31u;
    const uint32_t rb = (instruction >> 11) & 31u;
    const uint32_t xo = (instruction >> 1) & 1023u;

    if (cpu == 0) return PPC_STEP_UNSUPPORTED;

    /* I-form unconditional branch. Displacement is a signed 26-bit value
     * whose low two bits are zero; AA selects absolute vs relative target. */
    if (opcode == 18u) {
        const uint32_t raw_disp = instruction & 0x03fffffcu;
        const int64_t displacement = (raw_disp & 0x02000000u)
            ? (int64_t)raw_disp - 0x04000000LL
            : (int64_t)raw_disp;
        const uint32_t old_pc = cpu->pc;
        const uint32_t target = (instruction & 2u)
            ? (uint32_t)displacement
            : old_pc + (uint32_t)displacement;
        if ((instruction & 1u) != 0u) cpu->lr = old_pc + 4u;
        cpu->pc = target;
        return PPC_STEP_OK;
    }

    /* D-form loads/stores use a signed displacement and RA=0 as base zero. */
    if (opcode == 32u || opcode == 34u || opcode == 36u || opcode == 38u ||
        opcode == 40u || opcode == 44u) {
        const uint32_t raw_disp = instruction & 0xffffu;
        const int32_t disp = (raw_disp & 0x8000u)
            ? (int32_t)raw_disp - 65536
            : (int32_t)raw_disp;
        const uint32_t base = (ra == 0u) ? 0u : cpu->gpr[ra];
        const uint32_t address = base + (uint32_t)disp;
        if (memory == 0) return PPC_STEP_UNSUPPORTED;
        if ((opcode == 32u || opcode == 36u) && (address & 3u) != 0u)
            return PPC_STEP_ALIGNMENT;
        if ((opcode == 40u || opcode == 44u) && (address & 1u) != 0u)
            return PPC_STEP_ALIGNMENT;
        if (opcode == 32u) {
            uint32_t value;
            if (!wii_memory_read32be(memory, address, &value)) return PPC_STEP_UNSUPPORTED;
            cpu->gpr[rt] = value;
        } else if (opcode == 34u) {
            uint8_t value;
            if (!wii_memory_read8(memory, address, &value)) return PPC_STEP_UNSUPPORTED;
            cpu->gpr[rt] = value;
        } else if (opcode == 40u) {
            uint16_t value;
            if (!wii_memory_read16be(memory, address, &value)) return PPC_STEP_UNSUPPORTED;
            cpu->gpr[rt] = (uint32_t)value;
        } else if (opcode == 36u) {
            if (!wii_memory_write32be(memory, address, cpu->gpr[rt])) return PPC_STEP_UNSUPPORTED;
        } else if (opcode == 44u) {
            if (!wii_memory_write16be(memory, address, (uint16_t)cpu->gpr[rt])) return PPC_STEP_UNSUPPORTED;
        } else {
            if (!wii_memory_write8(memory, address, (uint8_t)cpu->gpr[rt])) return PPC_STEP_UNSUPPORTED;
        }
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

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
        if (xo == 266u && (instruction & 1u) == 0u) {
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
        if (xo == 8u && (instruction & 1u) == 0u) {
            cpu->gpr[rt] = ppc_subfc(&cpu->alu, cpu->gpr[ra], cpu->gpr[rb]);
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }
    }

    return PPC_STEP_UNSUPPORTED;
}


ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction)
{
    return ppc_cpu_step_with_memory(cpu, instruction, 0);
}
