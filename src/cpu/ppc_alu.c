#include "ppc/ppc_alu.h"

uint32_t ppc_subfc(ppc_alu_state_t *state, uint32_t ra, uint32_t rb)
{
    const uint32_t result = (uint32_t)((uint64_t)rb - (uint64_t)ra);
    if (state != 0) {
        state->xer_ca = (rb >= ra) ? 1u : 0u;
    }
    return result;
}

int ppc_compare_unsigned(uint32_t a, uint32_t b)
{
    return (a > b) - (a < b);
}

int ppc_compare_signed(int32_t a, int32_t b)
{
    return (a > b) - (a < b);
}
