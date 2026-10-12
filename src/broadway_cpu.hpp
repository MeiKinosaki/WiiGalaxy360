#pragma once
#include "memory_bus.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

class BroadwayCpu {
public:
    explicit BroadwayCpu(MemoryBus& bus) noexcept;
    void reset() noexcept;
    bool step() noexcept;
    std::size_t run(std::size_t instruction_budget) noexcept;
    std::uint32_t pc() const noexcept;
    std::uint32_t reg(std::size_t index) const noexcept;
    bool halted() const noexcept;
private:
    MemoryBus& bus_;
    std::array<std::uint32_t, 32> gpr_{};
    std::uint32_t pc_ = 0;
    bool halted_ = false;
};
