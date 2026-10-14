"""Reference-level arithmetic tests for 32-bit PowerPC integer semantics.

These helpers are test oracles only; they are not connected to WiiGalaxy360's
CPU dispatcher. Run with: python3 tests/ppc_arithmetic_reference.py
"""
from dataclasses import dataclass
import unittest

MASK32 = 0xFFFFFFFF
MASK64 = 0xFFFFFFFFFFFFFFFF


@dataclass(frozen=True)
class XER:
    ca: int = 0
    ov: int = 0
    so: int = 0


def addc_reference(ra: int, rb: int, xer: XER = XER()):
    """addc: RT = RA + RB modulo 2**32; CA is the unsigned carry-out."""
    total = (ra & MASK32) + (rb & MASK32)
    return total & MASK32, XER(ca=1 if total > MASK32 else 0, ov=xer.ov, so=xer.so)


def subf_reference(ra: int, rb: int, xer: XER = XER()):
    """subf: RT = RB - RA modulo 2**32; XER is not changed."""
    return (rb - ra) & MASK32, xer


def subfe_reference(ra: int, rb: int, xer: XER = XER()):
    """subfe: RT = RB - RA - (1 - CA); CA reports no unsigned borrow."""
    ra &= MASK32
    rb &= MASK32
    borrow_in = 1 - (1 if xer.ca else 0)
    subtrahend = ra + borrow_in
    result = (rb - subtrahend) & MASK32
    ca = 1 if rb >= subtrahend else 0
    return result, XER(ca=ca, ov=xer.ov, so=xer.so)


class PPCArithmeticReferenceTests(unittest.TestCase):
    def test_addc_basic_sum(self):
        result, xer = addc_reference(12, 30)
        self.assertEqual(result, 42)
        self.assertEqual(xer.ca, 0)

    def test_addc_sets_carry_out(self):
        result, xer = addc_reference(MASK32, 1)
        self.assertEqual(result, 0)
        self.assertEqual(xer.ca, 1)

    def test_addc_maximum_plus_maximum(self):
        result, xer = addc_reference(MASK32, MASK32)
        self.assertEqual(result, MASK32 - 1)
        self.assertEqual(xer.ca, 1)

    def test_addc_preserves_other_xer_flags(self):
        _, xer = addc_reference(1, 2, XER(ca=1, ov=1, so=1))
        self.assertEqual(xer, XER(ca=0, ov=1, so=1))

    def test_subf_operand_order(self):
        result, xer = subf_reference(7, 20, XER(ca=1, ov=1, so=1))
        self.assertEqual(result, 13)
        self.assertEqual(xer, XER(ca=1, ov=1, so=1))

    def test_subf_wraps_at_32_bits(self):
        result, _ = subf_reference(1, 0)
        self.assertEqual(result, MASK32)

    def test_subfe_ca_one_means_no_incoming_borrow(self):
        result, xer = subfe_reference(4, 9, XER(ca=1))
        self.assertEqual(result, 5)
        self.assertEqual(xer.ca, 1)

    def test_subfe_ca_zero_consumes_borrow(self):
        result, xer = subfe_reference(4, 9, XER(ca=0))
        self.assertEqual(result, 4)
        self.assertEqual(xer.ca, 1)

    def test_subfe_borrow_clears_ca(self):
        result, xer = subfe_reference(9, 4, XER(ca=1))
        self.assertEqual(result, (4 - 9) & MASK32)
        self.assertEqual(xer.ca, 0)

    def test_subfe_preserves_ov_so(self):
        _, xer = subfe_reference(1, 2, XER(ca=1, ov=1, so=1))
        self.assertEqual(xer.ov, 1)
        self.assertEqual(xer.so, 1)

    def test_inputs_are_truncated_to_32_bits(self):
        result, xer = addc_reference(0x100000001, 1)
        self.assertEqual(result, 2)
        self.assertEqual(xer.ca, 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
