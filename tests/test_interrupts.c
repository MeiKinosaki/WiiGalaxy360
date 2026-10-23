#include <assert.h>
#include <stdint.h>
#include "wii/interrupts.h"
#include "ppc/ppc_cpu.h"

int main(void)
{
    wii_interrupt_controller_t ic;
    ppc_cpu_t cpu;
    uint32_t source = 99u;
    ppc_cpu_reset(&cpu, 0x80001000u);
    wii_interrupts_init(&ic);
    assert(wii_interrupts_pending(&ic) == 0u);
    wii_interrupts_raise(&ic, 4u);
    wii_interrupts_raise(&ic, 2u);
    assert(wii_interrupts_pending(&ic) == 0u); /* masked by default */
    wii_interrupts_enable(&ic, (1u << 2) | (1u << 4));
    assert(wii_interrupts_pending(&ic) == ((1u << 2) | (1u << 4)));
    assert(!ppc_cpu_poll_external_interrupt(&cpu, wii_interrupts_pending(&ic), 0));
    assert(cpu.pc == 0x80001000u);
    assert(ppc_cpu_poll_external_interrupt(&cpu, wii_interrupts_pending(&ic), 1));
    assert(cpu.pc == 0x500u && cpu.srr0 == 0x80001000u);
    assert(wii_interrupts_pending(&ic) == ((1u << 2) | (1u << 4)));
    assert(wii_interrupts_next(&ic, &source) && source == 2u);
    wii_interrupts_clear(&ic, 2u);
    assert(wii_interrupts_next(&ic, &source) && source == 4u);
    wii_interrupts_disable(&ic, 1u << 4);
    assert(wii_interrupts_pending(&ic) == 0u);
    assert(!wii_interrupts_next(&ic, &source));
    wii_interrupts_raise(&ic, 32u); /* invalid source ignored */
    assert(wii_interrupts_pending(&ic) == 0u);
    assert(!wii_interrupts_next(&ic, 0));
    return 0;
}
