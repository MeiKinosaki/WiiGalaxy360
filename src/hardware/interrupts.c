#include "wii/interrupts.h"

void wii_interrupts_init(wii_interrupt_controller_t *controller)
{
    if (controller == 0) return;
    controller->pending = 0u;
    controller->enabled = 0u;
}

void wii_interrupts_raise(wii_interrupt_controller_t *controller, uint32_t source)
{
    if (controller != 0 && source < 32u)
        controller->pending |= (uint32_t)1u << source;
}

void wii_interrupts_clear(wii_interrupt_controller_t *controller, uint32_t source)
{
    if (controller != 0 && source < 32u)
        controller->pending &= ~((uint32_t)1u << source);
}

void wii_interrupts_enable(wii_interrupt_controller_t *controller, uint32_t mask)
{
    if (controller != 0) controller->enabled |= mask;
}

void wii_interrupts_disable(wii_interrupt_controller_t *controller, uint32_t mask)
{
    if (controller != 0) controller->enabled &= ~mask;
}

uint32_t wii_interrupts_pending(const wii_interrupt_controller_t *controller)
{
    return controller == 0 ? 0u : controller->pending & controller->enabled;
}

bool wii_interrupts_next(const wii_interrupt_controller_t *controller, uint32_t *source)
{
    uint32_t active;
    uint32_t bit;
    if (controller == 0 || source == 0) return false;
    active = wii_interrupts_pending(controller);
    if (active == 0u) return false;
    /* Deterministic priority: lowest numbered enabled pending source first. */
    for (bit = 0u; bit < 32u; ++bit) {
        if ((active & ((uint32_t)1u << bit)) != 0u) {
            *source = bit;
            return true;
        }
    }
    return false;
}
