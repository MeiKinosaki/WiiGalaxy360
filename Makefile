CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2
CPPFLAGS ?= -Iinclude

.PHONY: test test-ppc-alu test-ppc-cpu clean

test: test-ppc-alu test-ppc-cpu

test-ppc-alu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c tests/test_ppc_alu.c -o test_ppc_alu
	./test_ppc_alu

test-ppc-cpu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c src/cpu/ppc_cpu.c tests/test_ppc_cpu.c -o test_ppc_cpu
	./test_ppc_cpu

clean:
	rm -f test_ppc_alu test_ppc_cpu
