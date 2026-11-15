CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2
CPPFLAGS ?= -Iinclude

.PHONY: test test-ppc-alu test-ppc-cpu test-ppc-fpu test-gx-fifo test-memory test-mmu test-state test-interrupts clean

test: test-ppc-alu test-ppc-cpu test-ppc-fpu test-gx-fifo test-memory test-mmu test-state test-interrupts

test-ppc-alu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c tests/test_ppc_alu.c -o test_ppc_alu
	./test_ppc_alu

test-ppc-cpu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c src/cpu/ppc_cpu.c src/memory/memory.c src/memory/mmu.c tests/test_ppc_cpu.c -o test_ppc_cpu
	./test_ppc_cpu

test-ppc-fpu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_fpu.c tests/test_ppc_fpu.c -lm -o test_ppc_fpu
	./test_ppc_fpu

test-gx-fifo:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/hardware/gx_fifo.c tests/test_gx_fifo.c -o test_gx_fifo
	./test_gx_fifo

test-memory:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/memory/memory.c tests/test_memory.c -o test_memory
	./test_memory

test-mmu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/memory/memory.c src/memory/mmu.c tests/test_mmu.c -o test_mmu
	./test_mmu

test-state:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c src/cpu/ppc_cpu.c src/memory/memory.c src/memory/mmu.c src/state/state.c tests/test_state.c -o test_state
	./test_state

test-interrupts:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c src/cpu/ppc_cpu.c src/memory/memory.c src/memory/mmu.c src/hardware/interrupts.c tests/test_interrupts.c -o test_interrupts
	./test_interrupts

clean:
	rm -f test_ppc_alu test_ppc_cpu test_memory test_mmu test_state test_state.bin test_interrupts test_ppc_fpu test_gx_fifo
