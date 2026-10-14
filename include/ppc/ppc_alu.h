#ifndef WII_GALAXY360_PPC_ALU_H
#define WII_GALAXY360_PPC_ALU_H

#include <stdint.h>

typedef struct {
    uint32_t xer_ca;
} ppc_alu_state_t;

/* subfc RT,RA,RB: RT = RB - RA; CA=1 when no unsigned borrow. */
uint32_t ppc_subfc(ppc_alu_state_t *state, uint32_t ra, uint32_t rb);
int ppc_compare_unsigned(uint32_t a, uint32_t b);
int ppc_compare_signed(int32_t a, int32_t b);

#endif
