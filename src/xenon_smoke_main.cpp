// WiiGalaxy360 v2.13b: Broadway core startup diagnostic with structured fault output.
// Diagnostic only; this is not a Wii game loader or a complete emulator runtime.
#include "broadway_cpu.hpp"
#include "memory_bus.hpp"
#include <cstdint>
#include <cstdio>

namespace {
std::uint32_t dform(std::uint32_t opcode, std::uint32_t rt,
                    std::uint32_t ra, std::uint16_t immediate) {
    return (opcode << 26U) | (rt << 21U) | (ra << 16U) | immediate;
}
const char* fault_name(CpuFault fault) {
    switch (fault) {
    case CpuFault::None: return "none";
    case CpuFault::InstructionFetchOutOfRange: return "instruction-fetch-out-of-range";
    case CpuFault::DataReadOutOfRange: return "data-read-out-of-range";
    case CpuFault::DataWriteOutOfRange: return "data-write-out-of-range";
    case CpuFault::UnsupportedInstruction: return "unsupported-instruction";
    }
    return "unknown";
}
}

int main() {
    std::printf("WiiGalaxy360 v2.13b\n");
    std::printf("Runtime startup self-test...\n");

    MemoryBus bus(256U);
    const bool loaded =
        bus.write32(0U, dform(14U, 1U, 0U, 42U)) &&
        bus.write32(4U, dform(14U, 2U, 0U, 128U)) &&
        bus.write32(8U, dform(36U, 1U, 2U, 0U)) &&
        bus.write32(12U, dform(32U, 3U, 2U, 0U));
    if (!loaded) {
        std::printf("FAIL: could not load diagnostic program\n");
        return 10;
    }

    BroadwayCpu cpu(bus);
    const std::size_t executed = cpu.run(4U);
    std::uint32_t stored = 0U;
    const bool read_back = bus.read32(128U, stored);
    if (executed != 4U || cpu.halted() || cpu.fault() != CpuFault::None ||
        cpu.reg(3U) != 42U || !read_back || stored != 42U) {
        std::printf("FAIL: executed=%lu r3=%lu stored=%lu halted=%d fault=%s fault_pc=0x%08lx instruction=0x%08lx\n",
                    static_cast<unsigned long>(executed),
                    static_cast<unsigned long>(cpu.reg(3U)),
                    static_cast<unsigned long>(stored), cpu.halted() ? 1 : 0,
                    fault_name(cpu.fault()),
                    static_cast<unsigned long>(cpu.fault_pc()),
                    static_cast<unsigned long>(cpu.fault_instruction()));
        return 11;
    }

    std::printf("PASS: Broadway addi/stw/lwz; executed=%lu r3=%lu pc=0x%08lx fault=%s\n",
                static_cast<unsigned long>(executed),
                static_cast<unsigned long>(cpu.reg(3U)),
                static_cast<unsigned long>(cpu.pc()), fault_name(cpu.fault()));
    return 0;
}
