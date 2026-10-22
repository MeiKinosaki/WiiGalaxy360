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

static uint32_t encode_addc(unsigned rt, unsigned ra, unsigned rb)
{
    return (31u << 26) | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           ((uint32_t)rb << 11) | (10u << 1);
}

static uint32_t encode_addic(unsigned rt, unsigned ra, uint16_t simm)
{
    return (12u << 26) | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           (uint32_t)simm;
}

static uint32_t encode_spr(unsigned xo, unsigned rt, unsigned spr)
{
    return (31u << 26) | ((uint32_t)rt << 21) |
           ((spr & 31u) << 16) | (((spr >> 5) & 31u) << 11) |
           (xo << 1);
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

static uint32_t encode_cond_branch(unsigned bo, unsigned bi, uint16_t displacement,
                                   unsigned absolute, unsigned link)
{
    return (16u << 26) | ((uint32_t)bo << 21) | ((uint32_t)bi << 16) |
           ((uint32_t)displacement & 0xfffcu) |
           ((uint32_t)(absolute != 0u) << 1) | (uint32_t)(link != 0u);
}

static uint32_t encode_branch(uint32_t displacement, unsigned absolute, unsigned link)
{
    return (18u << 26) | (displacement & 0x03fffffcu) |
           ((uint32_t)(absolute != 0u) << 1) | (uint32_t)(link != 0u);
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

    /* X-form addc updates CA on unsigned carry-out and clears it otherwise. */
    ppc_cpu_reset(&cpu, 0x80003120u);
    cpu.gpr[4] = 0xffffffffu;
    cpu.gpr[5] = 1u;
    assert(ppc_cpu_step(&cpu, encode_addc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0u && cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003124u);
    cpu.gpr[4] = 10u;
    cpu.gpr[5] = 20u;
    assert(ppc_cpu_step(&cpu, encode_addc(6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 30u && cpu.alu.xer_ca == 0u);
    /* Restore the PC expected by the following legacy instruction sequence. */
    ppc_cpu_reset(&cpu, 0x80003124u);

    /* X-form neg computes two's-complement negation. */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 7u;
    assert(ppc_cpu_step(&cpu, encode_xform(104u, 6u, 4u, 0u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(0u - 7u));
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 0u;
    assert(ppc_cpu_step(&cpu, encode_xform(104u, 6u, 4u, 0u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0u);
    assert(ppc_cpu_step(&cpu, encode_xform(104u, 6u, 4u, 1u)) == PPC_STEP_UNSUPPORTED);

    /* X-form mullw keeps the low 32 bits of the signed product. */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 0xfffffffdu; /* -3 */
    cpu.gpr[5] = 7u;
    assert(ppc_cpu_step(&cpu, encode_xform(235u, 6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(0u - 21u));
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 0x40000000u;
    cpu.gpr[5] = 8u;
    assert(ppc_cpu_step(&cpu, encode_xform(235u, 6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0u); /* low-word wraparound */
    ppc_cpu_reset(&cpu, 0x80003124u);

    /* X-form nor writes bitwise NOT(RS | RB) to RA. */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 0x0f0f0f0fu;
    cpu.gpr[5] = 0x33333333u;
    assert(ppc_cpu_step(&cpu, encode_xform(124u, 6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0xc0c0c0c0u);
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 0u;
    cpu.gpr[5] = 0u;
    assert(ppc_cpu_step(&cpu, encode_xform(124u, 6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0xffffffffu);
    ppc_cpu_reset(&cpu, 0x80003124u);

    /* X-form subf computes RB - RA and must preserve XER[CA]. */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 9u;
    cpu.gpr[5] = 14u;
    cpu.alu.xer_ca = 1u;
    assert(ppc_cpu_step(&cpu, encode_xform(40u, 6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 5u && cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 15u;
    cpu.gpr[5] = 3u;
    cpu.alu.xer_ca = 0u;
    assert(ppc_cpu_step(&cpu, encode_xform(40u, 6u, 4u, 5u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(3u - 15u) && cpu.alu.xer_ca == 0u);
    ppc_cpu_reset(&cpu, 0x80003124u);

    /* D-form subfic sets CA iff unsigned immediate >= RA (no borrow). */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 3u;
    assert(ppc_cpu_step(&cpu, encode_dform(8u, 6u, 4u, 7u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 4u && cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 9u;
    assert(ppc_cpu_step(&cpu, encode_dform(8u, 6u, 4u, 7u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(7u - 9u) && cpu.alu.xer_ca == 0u);
    cpu.gpr[4] = 0xffffffffu;
    assert(ppc_cpu_step(&cpu, encode_dform(8u, 6u, 4u, 0xffffu)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0u && cpu.alu.xer_ca == 1u);
    ppc_cpu_reset(&cpu, 0x80003124u);

    /* X-form addze consumes XER[CA] as carry-in and updates carry-out. */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 0xffffffffu;
    cpu.alu.xer_ca = 1u;
    assert(ppc_cpu_step(&cpu, encode_xform(202u, 6u, 4u, 0u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0u && cpu.alu.xer_ca == 1u);
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 5u;
    cpu.alu.xer_ca = 0u;
    assert(ppc_cpu_step(&cpu, encode_xform(202u, 6u, 4u, 0u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 5u && cpu.alu.xer_ca == 0u);
    ppc_cpu_reset(&cpu, 0x80003124u);

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

    /* Move to/from the supported LR and CTR special-purpose registers. */
    ppc_cpu_reset(&cpu, 0x80003000u);
    cpu.lr = 0x12345678u;
    cpu.ctr = 0x87654321u;
    assert(ppc_cpu_step(&cpu, encode_spr(339u, 3u, 8u)) == PPC_STEP_OK);
    assert(cpu.gpr[3] == 0x12345678u && cpu.pc == 0x80003004u);
    assert(ppc_cpu_step(&cpu, encode_spr(339u, 4u, 9u)) == PPC_STEP_OK);
    assert(cpu.gpr[4] == 0x87654321u && cpu.pc == 0x80003008u);
    cpu.gpr[5] = 0xaabbccddu;
    assert(ppc_cpu_step(&cpu, encode_spr(467u, 5u, 8u)) == PPC_STEP_OK);
    assert(cpu.lr == 0xaabbccddu && cpu.pc == 0x8000300cu);
    cpu.gpr[6] = 0x0badf00du;
    assert(ppc_cpu_step(&cpu, encode_spr(467u, 6u, 9u)) == PPC_STEP_OK);
    assert(cpu.ctr == 0x0badf00du && cpu.pc == 0x80003010u);
    assert(ppc_cpu_step(&cpu, encode_spr(339u, 7u, 1u)) == PPC_STEP_UNSUPPORTED);
    assert(cpu.pc == 0x80003010u);

    /* Reset PC so the memory tests retain their own known starting point. */
    ppc_cpu_reset(&cpu, 0x80003134u);

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
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(44u, 19u, 0u, 2u), &memory) == PPC_STEP_OK);
        assert(ram[2] == 0x56u && ram[3] == 0x78u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(40u, 22u, 0u, 2u), &memory) == PPC_STEP_OK);
        assert(cpu.gpr[22] == 0x5678u);
        /* lha sign-extends the loaded halfword; lhz above zero-extends. */
        ram[0] = 0x80u;
        ram[1] = 0x01u;
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(42u, 24u, 0u, 0u), &memory) == PPC_STEP_OK);
        assert(cpu.gpr[24] == 0xffff8001u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(42u, 25u, 0u, 2u), &memory) == PPC_STEP_OK);
        assert(cpu.gpr[25] == 0x5678u);
        assert(cpu.pc == 0x80003154u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(32u, 23u, 0u, 5u), &memory) == PPC_STEP_ALIGNMENT);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(40u, 23u, 0u, 3u), &memory) == PPC_STEP_ALIGNMENT);
        assert(cpu.pc == 0x80003154u);
        assert(ppc_cpu_step_with_memory(&cpu, encode_dform(32u, 23u, 0u, 8u), &memory) == PPC_STEP_UNSUPPORTED);
        assert(cpu.pc == 0x80003154u);
    }

    /* Restore the baseline PC before the original unconditional-branch tests. */
    ppc_cpu_reset(&cpu, 0x80003154u);

    /* Unconditional relative branch with link stores the return address. */
    assert(ppc_cpu_step(&cpu, encode_branch(8u, 0u, 1u)) == PPC_STEP_OK);
    assert(cpu.pc == 0x8000315cu);
    assert(cpu.lr == 0x80003158u);

    /* Conditional branch: test CR bit, update PC, and optionally link. */
    ppc_cpu_reset(&cpu, 0x400u);
    cpu.cr = 0x80000000u;
    assert(ppc_cpu_step(&cpu, encode_cond_branch(12u, 0u, 8u, 0u, 1u)) == PPC_STEP_OK);
    assert(cpu.pc == 0x408u && cpu.lr == 0x404u);
    cpu.cr = 0u;
    assert(ppc_cpu_step(&cpu, encode_cond_branch(12u, 0u, 8u, 0u, 0u)) == PPC_STEP_OK);
    assert(cpu.pc == 0x40cu); /* not taken: fall through */
    ppc_cpu_reset(&cpu, 0x500u);
    cpu.ctr = 2u;
    assert(ppc_cpu_step(&cpu, encode_cond_branch(16u, 0u, 0xfffcu, 0u, 0u)) == PPC_STEP_OK);
    assert(cpu.ctr == 1u && cpu.pc == 0x4fcu); /* CTR nonzero */
    assert(ppc_cpu_step(&cpu, encode_cond_branch(16u, 0u, 0xfffcu, 0u, 0u)) == PPC_STEP_OK);
    assert(cpu.ctr == 0u && cpu.pc == 0x500u); /* CTR reached zero */

    /* Absolute branch changes PC to the encoded absolute displacement. */
    assert(ppc_cpu_step(&cpu, encode_branch(0x100u, 1u, 0u)) == PPC_STEP_OK);
    assert(cpu.pc == 0x100u);
    assert(cpu.lr == 0u); /* the intervening reset cleared LR */

    /* Unsupported instructions must not change PC. */
    assert(ppc_cpu_step(&cpu, 0u) == PPC_STEP_UNSUPPORTED);
    assert(cpu.pc == 0x100u);

    /* D-form mulli: signed immediate multiplication keeps low 32 bits. */
    ppc_cpu_reset(&cpu, 0x80003124u);
    cpu.gpr[4] = 7u;
    assert(ppc_cpu_step(&cpu, encode_dform(7u, 6u, 4u, 0xfffeu)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == (uint32_t)(0u - 14u));
    assert(cpu.pc == 0x80003128u);
    cpu.gpr[4] = 0x40000000u;
    assert(ppc_cpu_step(&cpu, encode_dform(7u, 6u, 4u, 8u)) == PPC_STEP_OK);
    assert(cpu.gpr[6] == 0u);
    ppc_cpu_reset(&cpu, 0x80003124u);

    /* Additional D-form coverage: addis shifts the signed immediate by 16,
     * while xori uses an unsigned immediate and treats GPR0 as a real register. */
    ppc_cpu_reset(&cpu, 0x200u);
    assert(ppc_cpu_step(&cpu, encode_dform(15u, 3u, 0u, 0x1234u)) == PPC_STEP_OK);
    assert(cpu.gpr[3] == 0x12340000u && cpu.pc == 0x204u);
    assert(ppc_cpu_step(&cpu, encode_dform(26u, 3u, 4u, 0xffffu)) == PPC_STEP_OK);
    assert(cpu.gpr[4] == 0x1234ffffu && cpu.pc == 0x208u);
    cpu.gpr[0] = 0xaaaaaaaa;
    assert(ppc_cpu_step(&cpu, encode_dform(26u, 0u, 0u, 0x00ffu)) == PPC_STEP_OK);
    assert(cpu.gpr[0] == 0xaaaaaa55u && cpu.pc == 0x20cu);
    /* cmpwi/cmplwi update the selected CR field using signed/unsigned rules. */
    ppc_cpu_reset(&cpu, 0x300u);
    cpu.gpr[5] = 0xffffffffu;
    cpu.cr = 0x12345678u;
    assert(ppc_cpu_step(&cpu, encode_dform(11u, 0u, 5u, 0xffffu)) == PPC_STEP_OK);
    assert((cpu.cr >> 28) == 2u); /* signed -1 equals immediate -1 */
    assert(cpu.pc == 0x304u);
    assert(ppc_cpu_step(&cpu, encode_dform(10u, 0u, 5u, 1u)) == PPC_STEP_OK);
    assert((cpu.cr >> 28) == 4u); /* unsigned 0xffffffff > 1 */
    assert((cpu.cr & 0x0fffffffu) == 0x02345678u);
    cpu.gpr[6] = 0x7fffffffu;
    assert(ppc_cpu_step(&cpu, encode_dform(11u, 0u, 6u, 0x8000u)) == PPC_STEP_OK);
    assert((cpu.cr >> 28) == 4u); /* signed max > -32768 */
    assert(ppc_cpu_step(&cpu, encode_dform(11u, 0u, 6u, 0u) | (1u << 21)) == PPC_STEP_UNSUPPORTED);
    assert(cpu.pc == 0x30cu);
    return 0;
}
