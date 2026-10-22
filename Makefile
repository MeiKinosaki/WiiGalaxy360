CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2
CPPFLAGS ?= -Iinclude

.PHONY: test test-ppc-alu test-ppc-cpu test-memory test-state test-interrupts clean

test: test-ppc-alu test-ppc-cpu test-memory test-state test-interrupts

test-ppc-alu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c tests/test_ppc_alu.c -o test_ppc_alu
	./test_ppc_alu

test-ppc-cpu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c src/cpu/ppc_cpu.c src/memory/memory.c tests/test_ppc_cpu.c -o test_ppc_cpu
	./test_ppc_cpu

test-memory:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/memory/memory.c tests/test_memory.c -o test_memory
	./test_memory

test-state:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c src/cpu/ppc_cpu.c src/memory/memory.c src/state/state.c tests/test_state.c -o test_state
	./test_state

test-interrupts:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/hardware/interrupts.c tests/test_interrupts.c -o test_interrupts
	./test_interrupts

clean:
	rm -f test_ppc_alu test_ppc_cpu test_memory test_state test_state.bin test_interrupts
