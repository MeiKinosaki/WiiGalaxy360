"""Reference-level tests for 32-bit PowerPC integer comparisons.

These helpers are test oracles only; they are not connected to the emulator CPU.
Run with: python3 tests/ppc_compare_reference.py
"""
import unittest

MASK32 = 0xFFFFFFFF
SIGN32 = 0x80000000


def signed32(value: int) -> int:
    value &= MASK32
    return value - 0x100000000 if value & SIGN32 else value


def compare_signed(ra: int, rb: int) -> int:
    """Return a CR field nibble: LT=8, GT=4, EQ=2 (SO omitted)."""
    a, b = signed32(ra), signed32(rb)
    return 8 if a < b else 4 if a > b else 2


def compare_unsigned(ra: int, rb: int) -> int:
    """Return a CR field nibble: LT=8, GT=4, EQ=2 (SO omitted)."""
    a, b = ra & MASK32, rb & MASK32
    return 8 if a < b else 4 if a > b else 2


class PPCCompareReferenceTests(unittest.TestCase):
    def test_signed_less_than(self):
        self.assertEqual(compare_signed(3, 7), 8)

    def test_signed_greater_than(self):
        self.assertEqual(compare_signed(9, 2), 4)

    def test_signed_equal(self):
        self.assertEqual(compare_signed(0xFFFFFFFF, -1), 2)

    def test_signed_negative_vs_zero(self):
        self.assertEqual(compare_signed(0xFFFFFFFF, 0), 8)

    def test_signed_zero_vs_negative(self):
        self.assertEqual(compare_signed(0, 0xFFFFFFFF), 4)

    def test_signed_minimum_vs_maximum(self):
        self.assertEqual(compare_signed(0x80000000, 0x7FFFFFFF), 8)

    def test_unsigned_high_bit_is_large(self):
        self.assertEqual(compare_unsigned(0x80000000, 0x7FFFFFFF), 4)

    def test_unsigned_maximum_vs_zero(self):
        self.assertEqual(compare_unsigned(MASK32, 0), 4)

    def test_unsigned_zero_vs_maximum(self):
        self.assertEqual(compare_unsigned(0, MASK32), 8)

    def test_unsigned_equal_with_truncation(self):
        self.assertEqual(compare_unsigned(0x100000001, 1), 2)

    def test_signed_equal_with_truncation(self):
        self.assertEqual(compare_signed(0x100000001, 1), 2)

    def test_result_is_only_cr_comparison_bits(self):
        self.assertIn(compare_signed(5, 5), (8, 4, 2))
        self.assertIn(compare_unsigned(5, 5), (8, 4, 2))


if __name__ == "__main__":
    unittest.main(verbosity=2)
