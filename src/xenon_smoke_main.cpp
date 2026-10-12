// WiiGalaxy360 v2.12b: deterministic Broadway-core startup self-test.
// This is a runtime diagnostic, not a Wii game loader or full emulator entry point.
#include "broadway_cpu.hpp"
#include "memory_bus.hpp"
#include <cstdint>
#include <cstdio>

namespace {
std::uint32_t dform(std::uint32_t opcode, std::uint32_t rt,
                    std::uint32_t ra, std::uint16_t immediate) {
    return (opcode << 26U) | (rt << 21U) | (ra << 16U) | immediate;
}
}

int main() {
    std::printf("WiiGalaxy360 v2.12b\\n");
    std::printf("Runtime startup self-test...\\n");

    MemoryBus bus(256U);
    const bool loaded =
        bus.write32(0U, dform(14U, 1U, 0U, 42U)) &&   // addi r1,0,42
        bus.write32(4U, dform(14U, 2U, 0U, 128U)) &&  // addi r2,0,128
        bus.write32(8U, dform(36U, 1U, 2U, 0U)) &&    // stw r1,0(r2)
        bus.write32(12U, dform(32U, 3U, 2U, 0U));     // lwz r3,0(r2)
    if (!loaded) {
        std::printf("FAIL: could not load diagnostic program\\n");
        return 10;
    }

    BroadwayCpu cpu(bus);
    const std::size_t executed = cpu.run(4U);
    std::uint32_t stored = 0U;
    const bool read_back = bus.read32(128U, stored);
    if (executed != 4U || cpu.halted() || cpu.reg(3U) != 42U ||
        !read_back || stored != 42U) {
        std::printf("FAIL: executed=%lu r3=%lu stored=%lu halted=%d\\n",
                    static_cast<unsigned long>(executed),
                    static_cast<unsigned long>(cpu.reg(3U)),
                    static_cast<unsigned long>(stored), cpu.halted() ? 1 : 0);
        return 11;
    }

    std::printf("PASS: Broadway addi/stw/lwz; executed=%lu r3=%lu\\n",
                static_cast<unsigned long>(executed),
                static_cast<unsigned long>(cpu.reg(3U)));
    return 0;
}
