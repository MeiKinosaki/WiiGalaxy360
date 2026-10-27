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
    cpu->srr0 = 0u;
    cpu->srr1 = 0u;
    cpu->dar = 0u;
    cpu->dsisr = 0u;
    cpu->exception_vector = 0u;
    cpu->exception_pending = 0u;
}

void ppc_cpu_raise_exception(ppc_cpu_t *cpu, ppc_exception_t exception,
                             uint32_t fault_pc, uint32_t fault_address)
{
    uint32_t vector;
    if (cpu == 0) return;
    switch (exception) {
    case PPC_EXCEPTION_RESET: vector = 0x100u; break;
    case PPC_EXCEPTION_MACHINE_CHECK: vector = 0x200u; break;
    case PPC_EXCEPTION_DATA_STORAGE: vector = 0x300u; break;
    case PPC_EXCEPTION_INSTRUCTION_STORAGE: vector = 0x400u; break;
    case PPC_EXCEPTION_EXTERNAL: vector = 0x500u; break;
    case PPC_EXCEPTION_ALIGNMENT: vector = 0x600u; break;
    case PPC_EXCEPTION_PROGRAM: vector = 0x700u; break;
    case PPC_EXCEPTION_SYSTEM_CALL: vector = 0xc00u; break;
    default: return;
    }
    /* MSR is not modelled yet; SRR1 records zero until privileged state lands. */
    cpu->srr0 = fault_pc;
    cpu->srr1 = 0u;
    cpu->dar = fault_address;
    cpu->dsisr = (exception == PPC_EXCEPTION_DATA_STORAGE) ? 0x02000000u : 0u;
    cpu->exception_vector = vector;
    cpu->exception_pending = (uint32_t)exception + 1u;
    cpu->pc = vector;
}

int ppc_cpu_poll_external_interrupt(ppc_cpu_t *cpu, uint32_t active_pending,
                                     int external_interrupts_enabled)
{
    if (cpu == 0 || active_pending == 0u || !external_interrupts_enabled)
        return 0;
    ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_EXTERNAL, cpu->pc, 0u);
    return 1;
}

static ppc_step_result_t ppc_cpu_step_internal(ppc_cpu_t *cpu, uint32_t instruction, wii_memory_t *memory, wii_bus_t *bus)
{
    const uint32_t opcode = instruction >> 26;
    const uint32_t rt = (instruction >> 21) & 31u;
    const uint32_t ra = (instruction >> 16) & 31u;
    const uint32_t rb = (instruction >> 11) & 31u;
    const uint32_t xo = (instruction >> 1) & 1023u;

    if (cpu == 0) return PPC_STEP_UNSUPPORTED;

    /* B-form conditional branch. BO controls CR and CTR tests; BD is signed
     * and word-aligned. This subset does not model branch prediction hints. */
    if (opcode == 16u) {
        const uint32_t bo = (instruction >> 21) & 31u;
        const uint32_t bi = (instruction >> 16) & 31u;
        const uint32_t raw_disp = instruction & 0x0000fffcu;
        const int32_t displacement = (raw_disp & 0x00008000u)
            ? (int32_t)raw_disp - 0x00010000
            : (int32_t)raw_disp;
        const uint32_t old_pc = cpu->pc;
        int cr_ok = 1;
        int ctr_ok = 1;
        if ((bo & 0x10u) == 0u) {
            const uint32_t cr_bit = (cpu->cr >> (31u - bi)) & 1u;
            cr_ok = cr_bit == ((bo & 0x08u) != 0u ? 1u : 0u);
        }
        if ((bo & 0x04u) == 0u) {
            cpu->ctr -= 1u;
            ctr_ok = ((cpu->ctr != 0u) != ((bo & 0x02u) != 0u));
        }
        if (cr_ok && ctr_ok) {
            const uint32_t target = (instruction & 2u)
                ? (uint32_t)displacement
                : old_pc + (uint32_t)displacement;
            if ((instruction & 1u) != 0u) cpu->lr = old_pc + 4u;
            cpu->pc = target;
        } else {
            cpu->pc = old_pc + 4u;
        }
        return PPC_STEP_OK;
    }

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
        opcode == 40u || opcode == 42u || opcode == 44u) {
        const uint32_t raw_disp = instruction & 0xffffu;
        const int32_t disp = (raw_disp & 0x8000u)
            ? (int32_t)raw_disp - 65536
            : (int32_t)raw_disp;
        const uint32_t base = (ra == 0u) ? 0u : cpu->gpr[ra];
        const uint32_t address = base + (uint32_t)disp;
        if (memory == 0 && bus == 0) return PPC_STEP_UNSUPPORTED;
        if ((opcode == 32u || opcode == 36u) && (address & 3u) != 0u) {
            ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_ALIGNMENT, cpu->pc, address);
            return PPC_STEP_ALIGNMENT;
        }
        if ((opcode == 40u || opcode == 42u || opcode == 44u) && (address & 1u) != 0u) {
            ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_ALIGNMENT, cpu->pc, address);
            return PPC_STEP_ALIGNMENT;
        }
        if (opcode == 32u) {
            uint32_t value;
            if (!(bus != 0 ? wii_bus_read32be(bus, address, &value) : wii_memory_read32be(memory, address, &value))) { ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_DATA_STORAGE, cpu->pc, address); return PPC_STEP_UNSUPPORTED; }
            cpu->gpr[rt] = value;
        } else if (opcode == 34u) {
            uint8_t value;
            if (!(bus != 0 ? wii_bus_read8(bus, address, &value) : wii_memory_read8(memory, address, &value))) { ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_DATA_STORAGE, cpu->pc, address); return PPC_STEP_UNSUPPORTED; }
            cpu->gpr[rt] = value;
        } else if (opcode == 40u || opcode == 42u) {
            uint16_t value;
            if (!(bus != 0 ? wii_bus_read16be(bus, address, &value) : wii_memory_read16be(memory, address, &value))) { ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_DATA_STORAGE, cpu->pc, address); return PPC_STEP_UNSUPPORTED; }
            cpu->gpr[rt] = (opcode == 42u && (value & 0x8000u) != 0u)
                ? (0xffff0000u | (uint32_t)value) : (uint32_t)value;
        } else if (opcode == 36u) {
            if (!(bus != 0 ? wii_bus_write32be(bus, address, cpu->gpr[rt]) : wii_memory_write32be(memory, address, cpu->gpr[rt]))) { ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_DATA_STORAGE, cpu->pc, address); return PPC_STEP_UNSUPPORTED; }
        } else if (opcode == 44u) {
            if (!(bus != 0 ? wii_bus_write16be(bus, address, (uint16_t)cpu->gpr[rt]) : wii_memory_write16be(memory, address, (uint16_t)cpu->gpr[rt]))) { ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_DATA_STORAGE, cpu->pc, address); return PPC_STEP_UNSUPPORTED; }
        } else {
            if (!(bus != 0 ? wii_bus_write8(bus, address, (uint8_t)cpu->gpr[rt]) : wii_memory_write8(memory, address, (uint8_t)cpu->gpr[rt]))) { ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_DATA_STORAGE, cpu->pc, address); return PPC_STEP_UNSUPPORTED; }
        }
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    /* D-form compare immediates: update one CR field with LT/GT/EQ.
     * L=1 requests a 64-bit compare and is unsupported by this 32-bit core. */
    if (opcode == 10u || opcode == 11u) {
        const uint32_t bf = (instruction >> 23) & 7u;
        const uint32_t ra_idx = (instruction >> 16) & 31u;
        const uint32_t raw_imm = instruction & 0xffffu;
        const uint32_t shift = 28u - (bf * 4u);
        uint32_t relation = 0u;
        if ((instruction & (1u << 21)) != 0u) return PPC_STEP_UNSUPPORTED;
        if (opcode == 11u) {
            const int32_t simm = (raw_imm & 0x8000u)
                ? (int32_t)raw_imm - 65536
                : (int32_t)raw_imm;
            const int32_t lhs = (int32_t)cpu->gpr[ra_idx];
            if (lhs < simm) relation = 8u;
            else if (lhs > simm) relation = 4u;
            else relation = 2u;
        } else {
            if (cpu->gpr[ra_idx] < raw_imm) relation = 8u;
            else if (cpu->gpr[ra_idx] > raw_imm) relation = 4u;
            else relation = 2u;
        }
        cpu->cr = (cpu->cr & ~(0xfu << shift)) | (relation << shift);
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    /* D-form addis: RT <- (RA == 0 ? 0 : GPR[RA]) + (sign_extend(SI) << 16). */
    if (opcode == 15u) {
        const uint32_t raw_imm = instruction & 0xffffu;
        const int32_t simm = (raw_imm & 0x8000u)
            ? (int32_t)raw_imm - 65536
            : (int32_t)raw_imm;
        const uint32_t base = (ra == 0u) ? 0u : cpu->gpr[ra];
        cpu->gpr[rt] = base + ((uint32_t)simm << 16);
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    /* D-form mulli: RT <- low 32 bits of signed RA * sign_extend(SI). */
    if (opcode == 7u) {
        const uint32_t raw_imm = instruction & 0xffffu;
        const int32_t simm = (raw_imm & 0x8000u)
            ? (int32_t)raw_imm - 65536
            : (int32_t)raw_imm;
        const int64_t product = (int64_t)(int32_t)cpu->gpr[ra] *
                                (int64_t)simm;
        cpu->gpr[rt] = (uint32_t)product;
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

    /* D-form subfic: RT <- sign_extend(SI) - RA; CA is set iff no borrow. */
    if (opcode == 8u) {
        const uint32_t raw_imm = instruction & 0xffffu;
        const int32_t simm = (raw_imm & 0x8000u)
            ? (int32_t)raw_imm - 65536
            : (int32_t)raw_imm;
        const uint32_t immediate = (uint32_t)simm;
        const uint32_t source = cpu->gpr[ra];
        cpu->gpr[rt] = immediate - source;
        cpu->alu.xer_ca = (uint32_t)(immediate >= source);
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

    /* D-form xori: RA <- RS ^ zero_extend(UI); RA=0 is a normal GPR. */
    if (opcode == 26u) {
        const uint32_t uimm = instruction & 0xffffu;
        cpu->gpr[ra] = cpu->gpr[rt] ^ uimm;
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

    /* M-form rlwinm: rotate RS left by SH, then apply the MB..ME mask.
     * PowerPC numbers mask bits from MSB=0; wrapped masks are supported. */
    if (opcode == 21u) {
        const uint32_t sh = (instruction >> 11) & 31u;
        const uint32_t mb = (instruction >> 6) & 31u;
        const uint32_t me = (instruction >> 1) & 31u;
        uint32_t rotated;
        uint32_t mask = 0u;
        uint32_t bit;
        if ((instruction & 1u) != 0u) return PPC_STEP_UNSUPPORTED;
        rotated = (cpu->gpr[rt] << sh) | (cpu->gpr[rt] >> ((32u - sh) & 31u));
        for (bit = 0u; bit < 32u; ++bit) {
            const int in_range = (mb <= me)
                ? (bit >= mb && bit <= me)
                : (bit >= mb || bit <= me);
            if (in_range) mask |= (1u << (31u - bit));
        }
        cpu->gpr[ra] = rotated & mask;
        cpu->pc += 4u;
        return PPC_STEP_OK;
    }

    if (opcode == 31u) {
        /* mfspr/mtspr subset: SPR encodes LR=8 and CTR=9. */
        if (xo == 339u || xo == 467u) {
            const uint32_t spr = ((instruction >> 16) & 31u) |
                                 (((instruction >> 11) & 31u) << 5);
            uint32_t *special = 0;
            if (spr == 8u) special = &cpu->lr;
            else if (spr == 9u) special = &cpu->ctr;
            if (special == 0 || (instruction & 1u) != 0u)
                return PPC_STEP_UNSUPPORTED;
            if (xo == 339u) cpu->gpr[rt] = *special;
            else *special = cpu->gpr[rt];
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form slw/srw use the low six bits of RB; shifts >= 32 yield zero. */
        if ((xo == 24u || xo == 536u) && (instruction & 1u) == 0u) {
            const uint32_t shift = cpu->gpr[rb] & 63u;
            if (shift >= 32u) cpu->gpr[ra] = 0u;
            else if (xo == 24u) cpu->gpr[ra] = cpu->gpr[rt] << shift;
            else cpu->gpr[ra] = cpu->gpr[rt] >> shift;
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form sraw/srawi: arithmetic right shift; CA records discarded
         * one-bits only when the source is negative. */
        if ((xo == 792u || xo == 824u) && (instruction & 1u) == 0u) {
            const uint32_t source = cpu->gpr[rt];
            const uint32_t shift = (xo == 824u) ? ((instruction >> 11) & 31u)
                                                : (cpu->gpr[rb] & 63u);
            const int negative = (source & 0x80000000u) != 0u;
            uint32_t result;
            uint32_t discarded = 0u;
            if (shift == 0u) {
                result = source;
            } else if (shift >= 32u) {
                result = negative ? 0xffffffffu : 0u;
                discarded = (source != 0xffffffffu) ? 1u : 0u;
            } else {
                const uint32_t mask = (1u << shift) - 1u;
                discarded = (source & mask) != 0u;
                result = source >> shift;
                if (negative) result |= (0xffffffffu << (32u - shift));
            }
            cpu->gpr[ra] = result;
            cpu->alu.xer_ca = (uint32_t)(negative && discarded);
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form addc: RT <- RA + RB, updating XER[CA] with carry-out. */
        if (xo == 10u && (instruction & 1u) == 0u) {
            cpu->gpr[rt] = ppc_addc(&cpu->alu, cpu->gpr[ra], cpu->gpr[rb]);
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form addze: RT <- RA + XER[CA], updating carry-out. */
        if (xo == 202u && (instruction & 1u) == 0u) {
            const uint64_t sum = (uint64_t)cpu->gpr[ra] +
                                 (uint64_t)(cpu->alu.xer_ca != 0u);
            cpu->gpr[rt] = (uint32_t)sum;
            cpu->alu.xer_ca = (uint32_t)(sum >> 32);
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form subfze: RT <- ~RA + XER[CA], updating carry-out. */
        if (xo == 200u && (instruction & 1u) == 0u) {
            const uint64_t sum = (uint64_t)(~cpu->gpr[ra]) +
                                 (uint64_t)(cpu->alu.xer_ca != 0u);
            cpu->gpr[rt] = (uint32_t)sum;
            cpu->alu.xer_ca = (uint32_t)(sum >> 32);
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

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

            if (xo == 124u) {
                cpu->gpr[ra] = ~(cpu->gpr[rt] | cpu->gpr[rb]);
                cpu->pc += 4u;
                return PPC_STEP_OK;
            }
        }

        /* X-form andc: RA <- RS & ~RB. */
        if (xo == 60u && (instruction & 1u) == 0u) {
            cpu->gpr[ra] = cpu->gpr[rt] & ~cpu->gpr[rb];
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form cntlzw: count leading zero bits in RS. */
        if (xo == 26u && rb == 0u && (instruction & 1u) == 0u) {
            uint32_t value = cpu->gpr[rt];
            uint32_t count = 0u;
            while (count < 32u && (value & (0x80000000u >> count)) == 0u)
                ++count;
            cpu->gpr[ra] = count;
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form extsb/extsh: sign-extend byte/halfword from RS. */
        if ((xo == 954u || xo == 922u) && rb == 0u &&
            (instruction & 1u) == 0u) {
            if (xo == 954u)
                cpu->gpr[ra] = (uint32_t)(int32_t)(int8_t)cpu->gpr[rt];
            else
                cpu->gpr[ra] = (uint32_t)(int32_t)(int16_t)cpu->gpr[rt];
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form neg: RT <- -RA; reserved RB must be zero. */
        if (xo == 104u && rb == 0u && (instruction & 1u) == 0u) {
            cpu->gpr[rt] = 0u - cpu->gpr[ra];
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form mullw: low 32 bits of signed RA * RB. */
        if (xo == 235u && (instruction & 1u) == 0u) {
            const int64_t product = (int64_t)(int32_t)cpu->gpr[ra] *
                                    (int64_t)(int32_t)cpu->gpr[rb];
            cpu->gpr[rt] = (uint32_t)product;
            cpu->pc += 4u;
            return PPC_STEP_OK;
        }

        /* X-form subf: RT <- RB - RA, preserving XER[CA]. */
        if (xo == 40u && (instruction & 1u) == 0u) {
            cpu->gpr[rt] = cpu->gpr[rb] - cpu->gpr[ra];
            cpu->pc += 4u;
            return PPC_STEP_OK;
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


ppc_step_result_t ppc_cpu_step_with_memory(ppc_cpu_t *cpu, uint32_t instruction, wii_memory_t *memory)
{
    return ppc_cpu_step_internal(cpu, instruction, memory, 0);
}

ppc_step_result_t ppc_cpu_step_with_bus(ppc_cpu_t *cpu, uint32_t instruction, wii_bus_t *bus)
{
    return ppc_cpu_step_internal(cpu, instruction, 0, bus);
}

ppc_step_result_t ppc_cpu_step(ppc_cpu_t *cpu, uint32_t instruction)
{
    return ppc_cpu_step_internal(cpu, instruction, 0, 0);
}

ppc_step_result_t ppc_cpu_step_from_memory(ppc_cpu_t *cpu, wii_memory_t *memory)
{
    uint32_t instruction;
    if (cpu == 0 || memory == 0) return PPC_STEP_UNSUPPORTED;
    if ((cpu->pc & 3u) != 0u) {
        ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_ALIGNMENT, cpu->pc, cpu->pc);
        return PPC_STEP_ALIGNMENT;
    }
    if (!wii_memory_read32be(memory, cpu->pc, &instruction)) {
        ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_INSTRUCTION_STORAGE, cpu->pc, cpu->pc);
        return PPC_STEP_UNSUPPORTED;
    }
    return ppc_cpu_step_internal(cpu, instruction, memory, 0);
}

ppc_step_result_t ppc_cpu_step_from_bus(ppc_cpu_t *cpu, wii_bus_t *bus)
{
    uint32_t instruction;
    if (cpu == 0 || bus == 0) return PPC_STEP_UNSUPPORTED;
    if ((cpu->pc & 3u) != 0u) {
        ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_ALIGNMENT, cpu->pc, cpu->pc);
        return PPC_STEP_ALIGNMENT;
    }
    if (!wii_bus_read32be(bus, cpu->pc, &instruction)) {
        ppc_cpu_raise_exception(cpu, PPC_EXCEPTION_INSTRUCTION_STORAGE, cpu->pc, cpu->pc);
        return PPC_STEP_UNSUPPORTED;
    }
    return ppc_cpu_step_internal(cpu, instruction, 0, bus);
}
