#include "ppc/ppc_alu.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    ppc_alu_state_t s = {0};
    assert(ppc_subfc(&s, 3u, 9u) == 6u && s.xer_ca == 1u);
    assert(ppc_subfc(&s, 9u, 3u) == UINT32_MAX - 5u && s.xer_ca == 0u);
    assert(ppc_subfc(&s, 0u, 0u) == 0u && s.xer_ca == 1u);
    assert(ppc_subfc(0, 1u, 2u) == 1u);
    assert(ppc_addc(&s, UINT32_MAX, 1u) == 0u && s.xer_ca == 1u);
    assert(ppc_addc(&s, 4u, 5u) == 9u && s.xer_ca == 0u);
    assert(ppc_compare_unsigned(1u, 2u) == -1);
    assert(ppc_compare_unsigned(2u, 2u) == 0);
    assert(ppc_compare_unsigned(UINT32_MAX, 1u) == 1);
    assert(ppc_compare_signed(-1, 1) == -1);
    assert(ppc_compare_signed(-4, -4) == 0);
    assert(ppc_compare_signed(7, -2) == 1);
    assert(ppc_and(0xF0u, 0x3Cu) == 0x30u);
    assert(ppc_or(0xF0u, 0x0Fu) == 0xFFu);
    assert(ppc_xor(0xFFu, 0x0Fu) == 0xF0u);
    puts("PPC ALU host tests: PASS");
    return 0;
}
