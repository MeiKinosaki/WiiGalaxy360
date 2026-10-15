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

static uint32_t encode_ori(unsigned rs, unsigned ra, uint16_t uimm)
{
    return (24u << 26) | ((uint32_t)rs << 21) | ((uint32_t)ra << 16) |
           (uint32_t)uimm;
}

static uint32_t encode_addic(unsigned rt, unsigned ra, uint16_t simm)
{
    return (12u << 26) | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           (uint32_t)simm;
}

static uint32_t encode_xform(unsigned xo, unsigned rt_rs, unsigned ra, unsigned rb)
{
    return (31u << 26) | ((uint32_t)rt_rs << 21) | ((uint32_t)ra << 16) |
           ((uint32_t)rb << 11) | ((uint32_t)xo << 1);
}

static uint32_t encode_dform(unsigned opcode, unsigned rt, unsigned ra, uint16_t disp)
{
    return ((uint32_t)opcode << 26) | ((uint32_t)rt << 21) |
           ((uint32_t)ra << 16) | (uint32_t)disp;
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

    /* ori zero-extends its immediate and writes the RA field. */
    cpu.cr = 0x87654321u;
    cpu.alu.xer_ca = 1u;
    assert(ppc_cpu_step(&cpu, encode_ori(6u, 10u, 0x8000u)) == PPC_STEP_OK);
    assert(cpu.gpr[10] == 0x807du);
    assert(cpu.cr == 0x87654321u && cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003110u);

    /* For ori, RS=0 reads GPR0; it is not addi's literal-zero special case. */
    assert(ppc_cpu_step(&cpu, encode_ori(0u, 9u, 0x00f0u)) == PPC_STEP_OK);
    assert(cpu.gpr[9] == 0xabcdeff1u);
    assert(cpu.pc == 0x80003114u);

    /* addic updates carry-out; 0xffffffff + 1 wraps and sets CA. */
    cpu.gpr[11] = 0xffffffffu;
    cpu.cr = 0x13572468u;
    assert(ppc_cpu_step(&cpu, encode_addic(12u, 11u, 1u)) == PPC_STEP_OK);
    assert(cpu.gpr[12] == 0u);
    assert(cpu.alu.xer_ca == 1u && cpu.cr == 0x13572468u);
    assert(cpu.pc == 0x80003118u);

    /* No carry-out when adding -1 to zero. */
    cpu.gpr[11] = 0u;
    assert(ppc_cpu_step(&cpu, encode_addic(12u, 11u, 0xffffu)) == PPC_STEP_OK);
    assert(cpu.gpr[12] == 0xffffffffu);
    assert(cpu.alu.xer_ca == 0u);
    assert(cpu.pc == 0x8000311cu);

    cpu.gpr[4] = 9u;
    cpu.gpr[5] = 14u;
    assert(ppc_cpu_step(&cpu, encode_subfc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 5u);
    assert(cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003120u);

    cpu.gpr[4] = 15u;
    cpu.gpr[5] = 3u;
    assert(ppc_cpu_step(&cpu, encode_subfc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(3u - 15u));
    assert(cpu.alu.xer_ca == 0u);

    /* X-form integer add and logical operations. */
    cpu.gpr[13] = 0x0f0f00ffu;
    cpu.gpr[14] = 0x00ff0f0fu;
    cpu.alu.xer_ca = 1u;
    assert(ppc_cpu_step(&cpu, encode_xform(266u, 15u, 13u, 14u)) == PPC_STEP_OK);
    assert(cpu.gpr[15] == 0x100e100eu);
    assert(cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003128u);

    assert(ppc_cpu_step(&cpu, encode_xform(28u, 13u, 16u, 14u)) == PPC_STEP_OK);
    assert(cpu.gpr[16] == 0x000f000fu);
    assert(cpu.pc == 0x8000312cu);

    assert(ppc_cpu_step(&cpu, encode_xform(444u, 13u, 17u, 14u)) == PPC_STEP_OK);
    assert(cpu.gpr[17] == 0x0fff0fffu);
    assert(cpu.pc == 0x80003130u);

    assert(ppc_cpu_step(&cpu, encode_xform(316u, 13u, 18u, 14u)) == PPC_STEP_OK);
    assert(cpu.gpr[18] == 0x0ff00ff0u);
    assert(cpu.pc == 0x80003134u);

    /* CPU loads/stores now use the bounded big-endian memory region. */
    {
        uint8_t ram[8] = {0};
        wii_memory_t memory = { ram, sizeof(ram) };
        cpu.gpr[19] = 0x12345678u;
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(36u, 19u, 0u, 4u), &memory) == PPC_STEP_OK);
        assert(ram[4] == 0x12u && ram[5] == 0x34u && ram[6] == 0x56u && ram[7] == 0x78u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(32u, 20u, 0u, 4u), &memory) == PPC_STEP_OK);
        assert(cpu.gpr[20] == 0x12345678u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(38u, 19u, 0u, 7u), &memory) == PPC_STEP_OK);
        assert(ram[7] == 0x78u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(34u, 21u, 0u, 7u), &memory) == PPC_STEP_OK);
        assert(cpu.gpr[21] == 0x78u);
        assert(cpu.pc == 0x80003144u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(32u, 22u, 0u, 6u), &memory) == PPC_STEP_UNSUPPORTED);
        assert(cpu.pc == 0x80003144u);
    }

    /* Unsupported instructions must not change PC. */
    assert(ppc_cpu_step(&cpu, 0u) == PPC_STEP_UNSUPPORTED);
    assert(cpu.pc == 0x80003144u);
    return 0;
}
