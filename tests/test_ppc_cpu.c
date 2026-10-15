#include <assert.h>
#include <stdint.h>
#include "ppc/ppc_cpu.h"

static uint32_t encode_subfc(unsigned rt, unsigned ra, unsigned rb)
{
    return (31u << 26) | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           ((uint32_t)rb << 11) | (8u << 1);
}

static uint32_t encode_addi(unsigned rt, unsigned ra, uint16_t simm)
{
    return (14u << 26) | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           (uint32_t)simm;
}

int main(void)
{
    ppc_cpu_t cpu;
    ppc_cpu_reset(&cpu, 0x80003100u);
    assert(cpu.pc == 0x80003100u);
    assert(cpu.gpr[0] == 0u && cpu.gpr[31] == 0u);
    assert(cpu.alu.xer_ca == 0u);

    /* addi with a positive immediate, preserving CR and XER[CA]. */
    cpu.gpr[4] = 100u;
    cpu.cr = 0x12345678u;
    cpu.alu.xer_ca = 1u;
    assert(ppc_cpu_step(&cpu, encode_addi(6u, 4u, 25u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 125u);
    assert(cpu.cr == 0x12345678u && cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003104u);

    /* Signed immediate: -3 must be sign-extended to 32 bits. */
    assert(ppc_cpu_step(&cpu, encode_addi(7u, 6u, 0xfffdu)) == PPC_STEP_OK);
    assert(cpu.gpr[7] == 122u);
    assert(cpu.pc == 0x80003108u);

    /* RA=0 means literal zero, not the current contents of GPR0. */
    cpu.gpr[0] = 0xabcdef01u;
    assert(ppc_cpu_step(&cpu, encode_addi(8u, 0u, 0xfffeu)) == PPC_STEP_OK);
    assert(cpu.gpr[8] == 0xfffffffeu);
    assert(cpu.pc == 0x8000310cu);

    cpu.gpr[4] = 9u;
    cpu.gpr[5] = 14u;
    assert(ppc_cpu_step(&cpu, encode_subfc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 5u);
    assert(cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003110u);

    cpu.gpr[4] = 15u;
    cpu.gpr[5] = 3u;
    assert(ppc_cpu_step(&cpu, encode_subfc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(3u - 15u));
    assert(cpu.alu.xer_ca == 0u);

    /* Unsupported instructions must not change PC. */
    assert(ppc_cpu_step(&cpu, 0u) == PPC_STEP_UNSUPPORTED);
    assert(cpu.pc == 0x80003114u);
    return 0;
}
