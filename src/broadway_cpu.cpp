#include "broadway_cpu.hpp"

BroadwayCpu::BroadwayCpu(MemoryBus& bus) noexcept : bus_(bus) { reset(); }

void BroadwayCpu::reset() noexcept {
    gpr_.fill(0);
    pc_ = 0;
    lr_ = 0;
    cr_ = 0;
    xer_ = 0;
    halted_ = false;
    fault_ = CpuFault::None;
    fault_pc_ = 0;
    fault_instruction_ = 0;
}

void BroadwayCpu::raise_fault(CpuFault reason, std::uint32_t instruction) noexcept {
    halted_ = true;
    fault_ = reason;
    fault_pc_ = pc_;
    fault_instruction_ = instruction;
}

bool BroadwayCpu::step() noexcept {
    if (halted_) return false;
    std::uint32_t instruction = 0;
    if (!bus_.read32(pc_, instruction)) {
        raise_fault(CpuFault::InstructionFetchOutOfRange, 0U);
        return false;
    }

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
        if (xo != 16U) {
            raise_fault(CpuFault::UnsupportedInstruction, instruction);
            return false;
        }
        const auto target = lr_ & ~std::uint32_t{3U};
        if ((instruction & 1U) != 0U) lr_ = next_pc;
        pc_ = target;
        return true;
    }
    case 24U: // ori RA,RS,UIMM; RS occupies the RT field
        gpr_[ra] = gpr_[rt] | static_cast<std::uint32_t>(immediate);
        break;
    case 31U: { // Selected X-form arithmetic/logical instructions, Rc=0 only.
        const auto rb = static_cast<std::size_t>((instruction >> 11U) & 31U);
        const auto xo = (instruction >> 1U) & 0x3ffU;
        std::uint32_t result = 0U;
        bool writes_rt = false;
        switch (xo) {
        case 266U: // add RT,RA,RB
            result = gpr_[ra] + gpr_[rb];
            writes_rt = true;
            break;
        case 40U: // subf RT,RA,RB => RB - RA
            result = gpr_[rb] - gpr_[ra];
            writes_rt = true;
            break;
        case 10U: { // addc RT,RA,RB
            const std::uint64_t sum = static_cast<std::uint64_t>(gpr_[ra]) +
                                      static_cast<std::uint64_t>(gpr_[rb]);
            result = static_cast<std::uint32_t>(sum);
            xer_ = (xer_ & ~0x20000000U) |
                   ((sum >> 32U) != 0U ? 0x20000000U : 0U);
            writes_rt = true;
            break;
        }
        case 8U: { // subfc RT,RA,RB => RB - RA, CA=not borrow
            const std::uint32_t minuend = gpr_[rb];
            const std::uint32_t subtrahend = gpr_[ra];
            result = minuend - subtrahend;
            xer_ = (xer_ & ~0x20000000U) |
                   (minuend >= subtrahend ? 0x20000000U : 0U);
            writes_rt = true;
            break;
        }
        case 316U: // xor RA,RS,RB; destination is RA
            result = gpr_[rt] ^ gpr_[rb];
            break;
        case 444U: // or RA,RS,RB; destination is RA
            result = gpr_[rt] | gpr_[rb];
            break;
        case 28U: // and RA,RS,RB; destination is RA
            result = gpr_[rt] & gpr_[rb];
            break;
        default:
            raise_fault(CpuFault::UnsupportedInstruction, instruction);
            return false;
        }
        if (writes_rt) gpr_[rt] = result;
        else gpr_[ra] = result;
        if ((instruction & 1U) != 0U) {
            // CR0 = LT, GT, EQ, SO. XER[SO] is not modelled yet.
            std::uint32_t field = result == 0U ? 0x2U :
                ((result & 0x80000000U) != 0U ? 0x8U : 0x4U);
            cr_ = (cr_ & 0x0fffffffU) | (field << 28U);
        }
        break;
    }
    case 32U: { // lwz RT,D(RA)
        const auto address = (ra == 0U ? 0U : gpr_[ra]) + static_cast<std::uint32_t>(simm);
        if (!bus_.read32(address, gpr_[rt])) {
            raise_fault(CpuFault::DataReadOutOfRange, instruction);
            return false;
        }
        break;
    }
    case 36U: { // stw RS,D(RA)
        const auto address = (ra == 0U ? 0U : gpr_[ra]) + static_cast<std::uint32_t>(simm);
        if (!bus_.write32(address, gpr_[rt])) {
            raise_fault(CpuFault::DataWriteOutOfRange, instruction);
            return false;
        }
        break;
    }
    default:
        raise_fault(CpuFault::UnsupportedInstruction, instruction);
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
std::uint32_t BroadwayCpu::cr() const noexcept { return cr_; }
std::uint32_t BroadwayCpu::xer() const noexcept { return xer_; }
bool BroadwayCpu::halted() const noexcept { return halted_; }
CpuFault BroadwayCpu::fault() const noexcept { return fault_; }
std::uint32_t BroadwayCpu::fault_pc() const noexcept { return fault_pc_; }
std::uint32_t BroadwayCpu::fault_instruction() const noexcept { return fault_instruction_; }
