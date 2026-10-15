#include <assert.h>
#include <stdint.h>
#include "ppc/ppc_cpu.h"

static uint32_t encode_subfc(unsigned rt, unsigned ra, unsigned rb)
{
    return (31u << 26) | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           ((uint32_t)rb << 11) | (8u << 1);
}

int main(void)
{
    ppc_cpu_t cpu;
    ppc_cpu_reset(&cpu, 0x80003100u);
    assert(cpu.pc == 0x80003100u);
    assert(cpu.gpr[0] == 0u && cpu.gpr[31] == 0u);
    assert(cpu.alu.xer_ca == 0u);

    cpu.gpr[4] = 9u;
    cpu.gpr[5] = 14u;
    assert(ppc_cpu_step(&cpu, encode_subfc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 5u);
    assert(cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003104u);

    cpu.gpr[4] = 15u;
    cpu.gpr[5] = 3u;
    assert(ppc_cpu_step(&cpu, encode_subfc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(3u - 15u));
    assert(cpu.alu.xer_ca == 0u);

    assert(ppc_cpu_step(&cpu, 0u) == PPC_STEP_UNSUPPORTED);
    assert(cpu.pc == 0x80003108u);
    return 0;
}
