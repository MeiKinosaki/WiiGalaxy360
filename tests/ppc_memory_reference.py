"""Reference tests for big-endian PowerPC memory access helpers.

These functions model byte-lane and alignment expectations for tests. They are
not connected to WiiGalaxy360's real CPU/memory bus implementation.
Run: python3 tests/ppc_memory_reference.py
"""
import unittest

MASK32 = 0xFFFFFFFF


def read_u16_be(memory: bytes, address: int) -> int:
    if address < 0 or address + 2 > len(memory):
        raise IndexError("memory read out of range")
    return int.from_bytes(memory[address:address + 2], "big")


def read_u32_be(memory: bytes, address: int) -> int:
    if address < 0 or address + 4 > len(memory):
        raise IndexError("memory read out of range")
    return int.from_bytes(memory[address:address + 4], "big")


def write_u16_be(memory: bytearray, address: int, value: int) -> None:
    if address < 0 or address + 2 > len(memory):
        raise IndexError("memory write out of range")
    memory[address:address + 2] = (value & 0xFFFF).to_bytes(2, "big")


def write_u32_be(memory: bytearray, address: int, value: int) -> None:
    if address < 0 or address + 4 > len(memory):
        raise IndexError("memory write out of range")
    memory[address:address + 4] = (value & MASK32).to_bytes(4, "big")


class PPCMemoryReferenceTests(unittest.TestCase):
    def test_u16_big_endian_read(self):
        self.assertEqual(read_u16_be(bytes([0x12, 0x34]), 0), 0x1234)

    def test_u32_big_endian_read(self):
        self.assertEqual(read_u32_be(bytes([0x12, 0x34, 0x56, 0x78]), 0), 0x12345678)

    def test_u16_big_endian_write(self):
        memory = bytearray(4)
        write_u16_be(memory, 1, 0xABCD)
        self.assertEqual(memory, bytearray([0, 0xAB, 0xCD, 0]))

    def test_u32_big_endian_write(self):
        memory = bytearray(4)
        write_u32_be(memory, 0, 0x89ABCDEF)
        self.assertEqual(memory, bytearray([0x89, 0xAB, 0xCD, 0xEF]))

    def test_u32_write_truncates_to_32_bits(self):
        memory = bytearray(4)
        write_u32_be(memory, 0, 0x123456789)
        self.assertEqual(read_u32_be(memory, 0), 0x23456789)

    def test_u16_read_rejects_negative_address(self):
        with self.assertRaises(IndexError):
            read_u16_be(bytes(4), -1)

    def test_u32_read_rejects_out_of_bounds(self):
        with self.assertRaises(IndexError):
            read_u32_be(bytes(4), 1)

    def test_u16_write_rejects_out_of_bounds(self):
        with self.assertRaises(IndexError):
            write_u16_be(bytearray(4), 3, 0xFFFF)

    def test_u32_write_rejects_out_of_bounds(self):
        with self.assertRaises(IndexError):
            write_u32_be(bytearray(4), 1, 0xFFFFFFFF)

    def test_adjacent_byte_lanes_remain_unchanged(self):
        memory = bytearray([0xAA, 0, 0, 0xBB])
        write_u16_be(memory, 1, 0x1234)
        self.assertEqual(memory, bytearray([0xAA, 0x12, 0x34, 0xBB]))


if __name__ == "__main__":
    unittest.main(verbosity=2)
