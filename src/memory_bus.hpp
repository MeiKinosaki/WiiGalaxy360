#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

class MemoryBus {
public:
    explicit MemoryBus(std::size_t size);
    std::size_t size() const noexcept;
    bool read8(std::uint32_t address, std::uint8_t& value) const noexcept;
    bool read32(std::uint32_t address, std::uint32_t& value) const noexcept;
    bool write8(std::uint32_t address, std::uint8_t value) noexcept;
    bool write32(std::uint32_t address, std::uint32_t value) noexcept;
    void clear() noexcept;
private:
    std::vector<std::uint8_t> memory_;
};
