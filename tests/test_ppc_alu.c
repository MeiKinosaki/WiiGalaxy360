#include "ppc/ppc_alu.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_subfc(void)
{
    ppc_alu_state_t state = {0};
    assert(ppc_subfc(&state, 3u, 9u) == 6u);
    assert(state.xer_ca == 1u);
    assert(ppc_subfc(&state, 9u, 3u) == UINT32_MAX - 5u);
    assert(state.xer_ca == 0u);
    assert(ppc_subfc(&state, 0u, 0u) == 0u);
    assert(state.xer_ca == 1u);
    assert(ppc_subfc(0, 1u, 2u) == 1u);
}

static void test_compare(void)
{
    assert(ppc_compare_unsigned(1u, 2u) == -1);
    assert(ppc_compare_unsigned(2u, 2u) == 0);
    assert(ppc_compare_unsigned(UINT32_MAX, 1u) == 1);
    assert(ppc_compare_signed(-1, 1) == -1);
    assert(ppc_compare_signed(-4, -4) == 0);
    assert(ppc_compare_signed(7, -2) == 1);
}

int main(void)
{
    test_subfc();
    test_compare();
    puts("PPC ALU host tests: PASS");
    return 0;
}
