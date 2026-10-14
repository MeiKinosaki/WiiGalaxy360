"""Reference checks for aligned 16/32-bit PowerPC-style memory accesses.

This is a test oracle only, not the WiiGalaxy360 memory bus or exception handler.
Run: python3 tests/ppc_alignment_reference.py
"""
import unittest


class AlignmentError(Exception):
    """Reference exception for a misaligned access."""


def check_access(memory_size: int, address: int, width: int) -> None:
    if width not in (1, 2, 4):
        raise ValueError("unsupported access width")
    if address < 0 or address + width > memory_size:
        raise IndexError("memory access out of range")
    if address % width:
        raise AlignmentError(f"{width}-byte access at misaligned address {address}")


class PPCAlignmentReferenceTests(unittest.TestCase):
    def test_byte_access_can_use_any_valid_address(self):
        check_access(16, 3, 1)

    def test_halfword_aligned_address(self):
        check_access(16, 4, 2)

    def test_word_aligned_address(self):
        check_access(16, 8, 4)

    def test_halfword_misalignment_is_rejected(self):
        with self.assertRaises(AlignmentError):
            check_access(16, 3, 2)

    def test_word_misalignment_by_one_is_rejected(self):
        with self.assertRaises(AlignmentError):
            check_access(16, 5, 4)

    def test_word_misalignment_by_two_is_rejected(self):
        with self.assertRaises(AlignmentError):
            check_access(16, 6, 4)

    def test_negative_address_is_out_of_range(self):
        with self.assertRaises(IndexError):
            check_access(16, -1, 1)

    def test_access_past_end_is_out_of_range(self):
        with self.assertRaises(IndexError):
            check_access(16, 14, 4)

    def test_exact_end_boundary_is_allowed(self):
        check_access(16, 12, 4)

    def test_unknown_width_is_rejected(self):
        with self.assertRaises(ValueError):
            check_access(16, 0, 8)


if __name__ == "__main__":
    unittest.main(verbosity=2)
