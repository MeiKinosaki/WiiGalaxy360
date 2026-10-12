#include "broadway_cpu.hpp"
#include "memory_bus.hpp"
#include <cstdint>
#include <iostream>

namespace {
int failures = 0;
void check(bool ok, const char* message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
constexpr std::uint32_t dform(std::uint32_t op, std::uint32_t rt,
                              std::uint32_t ra, std::uint16_t imm) {
    return (op << 26U) | (rt << 21U) | (ra << 16U) | imm;
}
void test_memory() {
    MemoryBus bus(64);
    std::uint32_t value = 0xdeadbeefU;
    check(bus.write32(8, 0x12345678U), "write32 valid address");
    check(bus.read32(8, value) && value == 0x12345678U, "big-endian read/write");
    check(!bus.read32(2, value), "unaligned read rejected");
    value = 0xdeadbeefU;
    check(!bus.read32(64, value) && value == 0xdeadbeefU, "failed read preserves output");
}
void test_cpu_load_store() {
    MemoryBus bus(256);
    check(bus.write32(0, dform(14, 1, 0, 16)), "write addi r1");
    check(bus.write32(4, dform(14, 2, 0, 0x1234)), "write addi r2");
    check(bus.write32(8, dform(36, 2, 1, 0)), "write stw");
    check(bus.write32(12, dform(32, 3, 1, 0)), "write lwz");
    BroadwayCpu cpu(bus);
    check(cpu.run(4) == 4, "execute supported instructions");
    check(cpu.reg(1) == 16, "addi r1");
    check(cpu.reg(2) == 0x1234, "addi r2");
    check(cpu.reg(3) == 0x1234, "lwz reads stored word");
    check(cpu.pc() == 16, "PC increments");
    check(!cpu.halted(), "CPU remains active");
}
void test_addis_and_branch() {
    MemoryBus bus(128);
    check(bus.write32(0, dform(15, 1, 0, 1)), "write addis r1");
    check(bus.write32(4, (18U << 26U) | 8U), "write branch forward by eight");
    check(bus.write32(8, dform(14, 2, 0, 99)), "write skipped addi");
    check(bus.write32(12, dform(14, 3, 0, 7)), "write target addi");
    BroadwayCpu cpu(bus);
    check(cpu.run(3) == 3, "execute addis, branch, and target instruction");
    check(cpu.reg(1) == 0x10000U, "addis shifts immediate by sixteen bits");
    check(cpu.reg(2) == 0U, "branch skips instruction");
    check(cpu.reg(3) == 7U, "branch reaches target");
    check(cpu.pc() == 16U, "PC reflects branch target execution");
}
void test_link_branch_stops() {
    MemoryBus bus(32);
    check(bus.write32(0, (18U << 26U) | 1U), "write branch with link bit");
    BroadwayCpu cpu(bus);
    check(cpu.run(1) == 0, "unsupported branch-link not counted");
    check(cpu.halted(), "branch-link halts until LR is implemented");
}
void test_halt_reset() {
    MemoryBus bus(32);
    check(bus.write32(0, 0xffffffffU), "write unsupported instruction");
    BroadwayCpu cpu(bus);
    check(cpu.run(1) == 0, "unsupported instruction not counted");
    check(cpu.halted(), "unsupported instruction halts prototype");
    cpu.reset();
    check(!cpu.halted() && cpu.pc() == 0, "reset clears halt");
}
}
int main() {
    test_memory(); test_cpu_load_store(); test_addis_and_branch();
    test_link_branch_stops(); test_halt_reset();
    if (failures) { std::cerr << failures << " assertion(s) failed\n"; return 1; }
    std::cout << "PASS: all host test assertions\n";
    return 0;
}
