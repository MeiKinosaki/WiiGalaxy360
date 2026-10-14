#include "memory_bus.hpp"
#include <algorithm>

MemoryBus::MemoryBus(std::size_t size) : memory_(size, 0) {}
std::size_t MemoryBus::size() const noexcept { return memory_.size(); }

bool MemoryBus::read8(std::uint32_t address, std::uint8_t& value) const noexcept {
    if (static_cast<std::uint64_t>(address) >= memory_.size()) return false;
    value = memory_[address];
    return true;
}

bool MemoryBus::read32(std::uint32_t address, std::uint32_t& value) const noexcept {
    const auto a = static_cast<std::uint64_t>(address);
    if ((address & 3U) != 0U || a + 4U > memory_.size()) return false;
    value = (static_cast<std::uint32_t>(memory_[address]) << 24U) |
            (static_cast<std::uint32_t>(memory_[address + 1U]) << 16U) |
            (static_cast<std::uint32_t>(memory_[address + 2U]) << 8U) |
            static_cast<std::uint32_t>(memory_[address + 3U]);
    return true;
}

bool MemoryBus::write8(std::uint32_t address, std::uint8_t value) noexcept {
    if (static_cast<std::uint64_t>(address) >= memory_.size()) return false;
    memory_[address] = value;
    return true;
}

bool MemoryBus::write32(std::uint32_t address, std::uint32_t value) noexcept {
    const auto a = static_cast<std::uint64_t>(address);
    if ((address & 3U) != 0U || a + 4U > memory_.size()) return false;
    memory_[address] = static_cast<std::uint8_t>(value >> 24U);
    memory_[address + 1U] = static_cast<std::uint8_t>(value >> 16U);
    memory_[address + 2U] = static_cast<std::uint8_t>(value >> 8U);
    memory_[address + 3U] = static_cast<std::uint8_t>(value);
    return true;
}
void MemoryBus::clear() noexcept { std::fill(memory_.begin(), memory_.end(), 0); }
