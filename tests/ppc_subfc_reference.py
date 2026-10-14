"""Reference-level tests for PowerPC subfc semantics.

This deliberately small model is not wired into the emulator core. It provides
an executable oracle for the arithmetic/carry contract while the actual CPU
implementation is located and integrated.
Run with: python3 tests/ppc_subfc_reference.py
"""
from dataclasses import dataclass
import unittest

MASK32 = 0xFFFFFFFF


@dataclass(frozen=True)
class XER:
    ca: int = 0
    ov: int = 0
    so: int = 0


def subfc_reference(ra: int, rb: int, xer: XER = XER()):
    """Model subfc RT,RA,RB: RT = RB - RA; CA means no unsigned borrow.

    Non-CA XER fields are preserved for the non-recording, non-OE instruction.
    Inputs are interpreted as 32-bit register values.
    """
    ra &= MASK32
    rb &= MASK32
    result = (rb - ra) & MASK32
    updated_xer = XER(ca=1 if rb >= ra else 0, ov=xer.ov, so=xer.so)
    return result, updated_xer


class SubfcReferenceTests(unittest.TestCase):
    def test_equal_operands_sets_carry(self):
        result, xer = subfc_reference(7, 7)
        self.assertEqual(result, 0)
        self.assertEqual(xer.ca, 1)

    def test_unsigned_no_borrow_sets_carry(self):
        result, xer = subfc_reference(3, 10)
        self.assertEqual(result, 7)
        self.assertEqual(xer.ca, 1)

    def test_unsigned_borrow_clears_carry(self):
        result, xer = subfc_reference(10, 3)
        self.assertEqual(result, (3 - 10) & MASK32)
        self.assertEqual(xer.ca, 0)

    def test_wraparound_is_32_bit(self):
        result, xer = subfc_reference(1, 0)
        self.assertEqual(result, MASK32)
        self.assertEqual(xer.ca, 0)

    def test_high_bit_values_are_unsigned_for_carry(self):
        result, xer = subfc_reference(0x80000000, 0x7FFFFFFF)
        self.assertEqual(result, 0xFFFFFFFF)
        self.assertEqual(xer.ca, 0)

    def test_preserves_ov_and_so(self):
        result, xer = subfc_reference(2, 5, XER(ca=0, ov=1, so=1))
        self.assertEqual(result, 3)
        self.assertEqual(xer, XER(ca=1, ov=1, so=1))

    def test_inputs_are_truncated_to_32_bits(self):
        result, xer = subfc_reference(0x100000001, 2)
        self.assertEqual(result, 1)
        self.assertEqual(xer.ca, 1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
