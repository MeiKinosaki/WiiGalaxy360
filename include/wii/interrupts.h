#ifndef WII_GALAXY360_INTERRUPTS_H
#define WII_GALAXY360_INTERRUPTS_H

#include <stdbool.h>
#include <stdint.h>

/* Generic interrupt-pending/mask controller for device integration.
 * Device-specific Wii interrupt routing and CPU MSR/EE delivery remain TODO. */
typedef struct {
    uint32_t pending;
    uint32_t enabled;
} wii_interrupt_controller_t;

void wii_interrupts_init(wii_interrupt_controller_t *controller);
void wii_interrupts_raise(wii_interrupt_controller_t *controller, uint32_t source);
void wii_interrupts_clear(wii_interrupt_controller_t *controller, uint32_t source);
void wii_interrupts_enable(wii_interrupt_controller_t *controller, uint32_t mask);
void wii_interrupts_disable(wii_interrupt_controller_t *controller, uint32_t mask);
uint32_t wii_interrupts_pending(const wii_interrupt_controller_t *controller);
bool wii_interrupts_next(const wii_interrupt_controller_t *controller, uint32_t *source);

#endif
