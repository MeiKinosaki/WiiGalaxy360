CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2
CPPFLAGS ?= -Iinclude

.PHONY: test test-ppc-alu clean

test: test-ppc-alu

test-ppc-alu:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/cpu/ppc_alu.c tests/test_ppc_alu.c -o test_ppc_alu
	./test_ppc_alu

clean:
	rm -f test_ppc_alu
