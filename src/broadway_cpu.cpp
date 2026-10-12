#include "broadway_cpu.hpp"

BroadwayCpu::BroadwayCpu(MemoryBus& bus) noexcept : bus_(bus) { reset(); }
void BroadwayCpu::reset() noexcept {
    gpr_.fill(0);
    pc_ = 0;
    lr_ = 0;
    halted_ = false;
}

bool BroadwayCpu::step() noexcept {
    if (halted_) return false;
    std::uint32_t instruction = 0;
    if (!bus_.read32(pc_, instruction)) { halted_ = true; return false; }

    const auto opcode = instruction >> 26U;
    const auto rt = static_cast<std::size_t>((instruction >> 21U) & 31U);
    const auto ra = static_cast<std::size_t>((instruction >> 16U) & 31U);
    const auto immediate = static_cast<std::uint16_t>(instruction & 0xffffU);
    const auto simm = static_cast<std::int32_t>(static_cast<std::int16_t>(immediate));
    const auto next_pc = pc_ + 4U;

    switch (opcode) {
    case 14U: // addi RT,RA,SIMM
        gpr_[rt] = (ra == 0U ? 0U : gpr_[ra]) + static_cast<std::uint32_t>(simm);
        break;
    case 15U: // addis RT,RA,SIMM
        gpr_[rt] = (ra == 0U ? 0U : gpr_[ra]) +
                   (static_cast<std::uint32_t>(simm) << 16U);
        break;
    case 18U: { // b/bl; partial branch support
        const auto link = (instruction & 1U) != 0U;
        std::int32_t displacement = static_cast<std::int32_t>(instruction & 0x03fffffcU);
        if ((displacement & 0x02000000) != 0)
            displacement |= static_cast<std::int32_t>(0xfc000000U);
        if (link) lr_ = next_pc;
        pc_ = (instruction & 2U) != 0U
            ? static_cast<std::uint32_t>(displacement)
            : pc_ + static_cast<std::uint32_t>(displacement);
        return true;
    }
    case 19U: { // bclr / blrl, extended opcode 16
        const auto xo = (instruction >> 1U) & 0x3ffU;
        if (xo != 16U) { halted_ = true; return false; }
        const auto target = lr_ & ~std::uint32_t{3U};
        if ((instruction & 1U) != 0U) lr_ = next_pc;
        pc_ = target;
        return true;
    }
    case 24U: // ori RA,RS,UIMM; RS occupies the RT field
        gpr_[ra] = gpr_[rt] | static_cast<std::uint32_t>(immediate);
        break;
    case 32U: { // lwz RT,D(RA)
        const auto address = (ra == 0U ? 0U : gpr_[ra]) + static_cast<std::uint32_t>(simm);
        if (!bus_.read32(address, gpr_[rt])) { halted_ = true; return false; }
        break;
    }
    case 36U: { // stw RS,D(RA)
        const auto address = (ra == 0U ? 0U : gpr_[ra]) + static_cast<std::uint32_t>(simm);
        if (!bus_.write32(address, gpr_[rt])) { halted_ = true; return false; }
        break;
    }
    default:
        halted_ = true;
        return false;
    }
    pc_ = next_pc;
    return true;
}

std::size_t BroadwayCpu::run(std::size_t instruction_budget) noexcept {
    std::size_t executed = 0;
    while (executed < instruction_budget && step()) ++executed;
    return executed;
}
std::uint32_t BroadwayCpu::pc() const noexcept { return pc_; }
std::uint32_t BroadwayCpu::reg(std::size_t index) const noexcept {
    return index < gpr_.size() ? gpr_[index] : 0U;
}
std::uint32_t BroadwayCpu::lr() const noexcept { return lr_; }
bool BroadwayCpu::halted() const noexcept { return halted_; }
