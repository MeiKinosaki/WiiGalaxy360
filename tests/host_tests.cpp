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
void test_call_and_return() {
    MemoryBus bus(128);
    check(bus.write32(0, dform(14, 1, 0, 1)), "write main instruction");
    check(bus.write32(4, (18U << 26U) | 12U | 1U), "write bl to function at 16");
    check(bus.write32(8, dform(14, 2, 0, 2)), "write caller continuation");
    check(bus.write32(12, 0x48000000U), "write guard branch");
    check(bus.write32(16, dform(14, 3, 0, 3)), "write function instruction");
    check(bus.write32(20, 0x4e800020U), "write blr");
    BroadwayCpu cpu(bus);
    check(cpu.run(5) == 5, "execute call, function and return");
    check(cpu.lr() == 8U, "bl stores return address in LR");
    check(cpu.reg(3) == 3U, "function body ran");
    check(cpu.reg(2) == 2U, "caller continues after blr");
    check(cpu.pc() == 12U, "return target is caller continuation");
    check(!cpu.halted(), "supported call/return remains active");
}
void test_link_branch_stops_no_longer_applicable() {
    MemoryBus bus(32);
    check(bus.write32(0, 0x4e800020U), "write blr with empty LR");
    BroadwayCpu cpu(bus);
    check(cpu.run(1) == 1, "blr is decoded as branch-to-LR");
    check(cpu.pc() == 0U, "empty LR returns to address zero in prototype");
}
void test_fault_diagnostics() {
    {
        MemoryBus bus(32);
        check(bus.write32(0, 0xffffffffU), "write invalid instruction");
        BroadwayCpu cpu(bus);
        check(cpu.run(1) == 0U, "invalid instruction is not counted");
        check(cpu.fault() == CpuFault::UnsupportedInstruction, "unsupported opcode has fault reason");
        check(cpu.fault_pc() == 0U, "unsupported opcode records fault PC");
        check(cpu.fault_instruction() == 0xffffffffU, "unsupported opcode records instruction");
        check(cpu.halted(), "fault halts CPU");
        check(cpu.run(1) == 0U, "halted CPU does not execute more instructions");
    }
    {
        MemoryBus bus(16);
        check(bus.write32(0, dform(32, 1, 0, 64)), "write out-of-range lwz");
        BroadwayCpu cpu(bus);
        check(cpu.run(1) == 0U, "out-of-range lwz is not counted");
        check(cpu.fault() == CpuFault::DataReadOutOfRange, "bad load reports data-read fault");
        check(cpu.fault_pc() == 0U, "bad load records PC");
        check(cpu.fault_instruction() == dform(32, 1, 0, 64), "bad load records instruction");
    }
    {
        MemoryBus bus(16);
        BroadwayCpu cpu(bus);
        check(cpu.step() == false, "fetch beyond empty instruction stream fails");
        check(cpu.fault() == CpuFault::InstructionFetchOutOfRange, "bad fetch reports fetch fault");
        check(cpu.fault_pc() == 0U, "bad fetch records PC");
    }
    {
        MemoryBus bus(32);
        check(bus.write32(0, dform(14, 1, 0, 1)), "write valid instruction before reset test");
        BroadwayCpu cpu(bus);
        check(cpu.run(1) == 1U, "valid instruction executes before reset");
        check(bus.write32(4, 0xffffffffU), "write invalid instruction for reset test");
        check(cpu.run(1) == 0U, "invalid instruction triggers reset test fault");
        cpu.reset();
        check(cpu.fault() == CpuFault::None && !cpu.halted() && cpu.pc() == 0U &&
              cpu.fault_pc() == 0U && cpu.fault_instruction() == 0U,
              "reset clears fault diagnostics");
    }
}
void test_halt_reset() {
    MemoryBus bus(32);
    check(bus.write32(0, 0xffffffffU), "write unsupported instruction");
    BroadwayCpu cpu(bus);
    check(cpu.run(1) == 0, "unsupported instruction not counted");
    check(cpu.halted(), "unsupported instruction halts prototype");
    check(cpu.fault() == CpuFault::UnsupportedInstruction, "halt has structured fault reason");
    cpu.reset();
    check(!cpu.halted() && cpu.pc() == 0 && cpu.lr() == 0, "reset clears halt and LR");
    check(cpu.fault() == CpuFault::None, "reset clears fault reason");
}
}
int main() {
    test_memory(); test_cpu_load_store(); test_addis_and_branch();
    test_call_and_return(); test_link_branch_stops_no_longer_applicable(); test_fault_diagnostics(); test_halt_reset();
    if (failures) { std::cerr << failures << " assertion(s) failed\n"; return 1; }
    std::cout << "PASS: all host test assertions\n";
    return 0;
}
