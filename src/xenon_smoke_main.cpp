// WiiGalaxy360 v2.15b: Broadway CR0 startup diagnostic.
// Diagnostic only; not a Wii game loader or complete emulator runtime.
#include "broadway_cpu.hpp"
#include "memory_bus.hpp"
#include <cstdint>
#include <cstdio>

namespace {
std::uint32_t dform(std::uint32_t opcode, std::uint32_t rt,
                    std::uint32_t ra, std::uint16_t immediate) {
    return (opcode << 26U) | (rt << 21U) | (ra << 16U) | immediate;
}
std::uint32_t xform(std::uint32_t rt, std::uint32_t ra,
                    std::uint32_t rb, std::uint32_t xo, bool record) {
    return (31U << 26U) | (rt << 21U) | (ra << 16U) |
           (rb << 11U) | (xo << 1U) | (record ? 1U : 0U);
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
    std::printf("WiiGalaxy360 v2.15b\n");
    std::printf("Broadway CR0 startup self-test...\n");
    MemoryBus bus(256U);
    const bool loaded =
        bus.write32(0U, dform(14U, 1U, 0U, 17U)) &&
        bus.write32(4U, dform(14U, 2U, 0U, 25U)) &&
        bus.write32(8U, xform(3U, 1U, 2U, 266U, true)) &&
        bus.write32(12U, dform(36U, 3U, 0U, 128U)) &&
        bus.write32(16U, dform(32U, 4U, 0U, 128U));
    if (!loaded) {
        std::printf("FAIL: could not load diagnostic program\n");
        return 10;
    }
    BroadwayCpu cpu(bus);
    const std::size_t executed = cpu.run(5U);
    std::uint32_t stored = 0U;
    const bool read_back = bus.read32(128U, stored);
    const std::uint32_t cr0 = (cpu.cr() >> 28U) & 0xfU;
    if (executed != 5U || cpu.halted() || cpu.fault() != CpuFault::None ||
        cpu.reg(4U) != 42U || !read_back || stored != 42U || cr0 != 0x4U) {
        std::printf("FAIL: executed=%lu r4=%lu stored=%lu CR0=0x%lx fault=%s pc=0x%08lx instruction=0x%08lx\n",
                    static_cast<unsigned long>(executed),
                    static_cast<unsigned long>(cpu.reg(4U)),
                    static_cast<unsigned long>(stored),
                    static_cast<unsigned long>(cr0), fault_name(cpu.fault()),
                    static_cast<unsigned long>(cpu.fault_pc()),
                    static_cast<unsigned long>(cpu.fault_instruction()));
        return 11;
    }
    std::printf("PASS: Broadway addi/add.Rc/stw/lwz; executed=%lu r4=%lu CR0=0x%lx fault=%s\n",
                static_cast<unsigned long>(executed),
                static_cast<unsigned long>(cpu.reg(4U)),
                static_cast<unsigned long>(cr0), fault_name(cpu.fault()));
    return 0;
}
