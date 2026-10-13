CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2
BUILD_DIR := build
HOST_TEST := $(BUILD_DIR)/host_tests
SOURCES := src/memory_bus.cpp src/broadway_cpu.cpp tests/host_tests.cpp

.PHONY: all host-test clean
all: host-test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(HOST_TEST): $(SOURCES) src/memory_bus.hpp src/broadway_cpu.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Isrc $(SOURCES) -o $@

host-test: $(HOST_TEST)
	./$(HOST_TEST)

clean:
	rm -rf $(BUILD_DIR)
