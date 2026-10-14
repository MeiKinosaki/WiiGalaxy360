#include "ppc/ppc_alu.h"

uint32_t ppc_subfc(ppc_alu_state_t *state, uint32_t ra, uint32_t rb)
{
    const uint32_t result = (uint32_t)((uint64_t)rb - (uint64_t)ra);
    if (state != 0) state->xer_ca = (rb >= ra) ? 1u : 0u;
    return result;
}

uint32_t ppc_addc(ppc_alu_state_t *state, uint32_t a, uint32_t b)
{
    const uint64_t sum = (uint64_t)a + (uint64_t)b;
    if (state != 0) state->xer_ca = (uint32_t)(sum >> 32);
    return (uint32_t)sum;
}

int ppc_compare_unsigned(uint32_t a, uint32_t b) { return (a > b) - (a < b); }
int ppc_compare_signed(int32_t a, int32_t b) { return (a > b) - (a < b); }
uint32_t ppc_and(uint32_t a, uint32_t b) { return a & b; }
uint32_t ppc_or(uint32_t a, uint32_t b) { return a | b; }
uint32_t ppc_xor(uint32_t a, uint32_t b) { return a ^ b; }
