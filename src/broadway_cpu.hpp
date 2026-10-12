#pragma once
#include "memory_bus.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

enum class CpuFault : std::uint8_t {
    None = 0,
    InstructionFetchOutOfRange,
    DataReadOutOfRange,
    DataWriteOutOfRange,
    UnsupportedInstruction
};

class BroadwayCpu {
public:
    explicit BroadwayCpu(MemoryBus& bus) noexcept;
    void reset() noexcept;
    bool step() noexcept;
    std::size_t run(std::size_t instruction_budget) noexcept;
    std::uint32_t pc() const noexcept;
    std::uint32_t reg(std::size_t index) const noexcept;
    std::uint32_t lr() const noexcept;
    bool halted() const noexcept;
    CpuFault fault() const noexcept;
    std::uint32_t fault_pc() const noexcept;
    std::uint32_t fault_instruction() const noexcept;
private:
    void raise_fault(CpuFault reason, std::uint32_t instruction) noexcept;
    MemoryBus& bus_;
    std::array<std::uint32_t, 32> gpr_{};
    std::uint32_t pc_ = 0;
    std::uint32_t lr_ = 0;
    bool halted_ = false;
    CpuFault fault_ = CpuFault::None;
    std::uint32_t fault_pc_ = 0;
    std::uint32_t fault_instruction_ = 0;
};
