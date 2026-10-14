"""Reference tests for classifying memory faults and exception metadata.

This is a small testing model, not a PowerPC exception handler and not wired to
WiiGalaxy360's actual CPU or memory bus.
Run: python3 tests/ppc_exception_reference.py
"""
from dataclasses import dataclass
import unittest


@dataclass(frozen=True)
class Fault:
    kind: str
    address: int
    width: int
    is_write: bool = False


def classify_fault(memory_size: int, fault: Fault) -> str:
    if fault.width not in (1, 2, 4):
        return "unsupported_width"
    if fault.address < 0 or fault.address + fault.width > memory_size:
        return "out_of_range"
    if fault.address % fault.width:
        return "misaligned"
    if fault.kind not in ("data", "instruction"):
        return "unknown_access_kind"
    return "none"


class PPCExceptionReferenceTests(unittest.TestCase):
    def test_valid_aligned_data_read(self):
        self.assertEqual(classify_fault(64, Fault("data", 8, 4)), "none")

    def test_valid_instruction_fetch_reference(self):
        self.assertEqual(classify_fault(64, Fault("instruction", 4, 4)), "none")

    def test_misaligned_halfword(self):
        self.assertEqual(classify_fault(64, Fault("data", 3, 2)), "misaligned")

    def test_misaligned_word(self):
        self.assertEqual(classify_fault(64, Fault("data", 6, 4)), "misaligned")

    def test_negative_address(self):
        self.assertEqual(classify_fault(64, Fault("data", -1, 1)), "out_of_range")

    def test_access_crosses_end(self):
        self.assertEqual(classify_fault(64, Fault("data", 62, 4)), "out_of_range")

    def test_unsupported_width(self):
        self.assertEqual(classify_fault(64, Fault("data", 0, 8)), "unsupported_width")

    def test_unknown_access_kind(self):
        self.assertEqual(classify_fault(64, Fault("dma", 0, 4)), "unknown_access_kind")

    def test_write_fault_metadata_is_preserved(self):
        fault = Fault("data", 3, 2, is_write=True)
        self.assertTrue(fault.is_write)
        self.assertEqual(classify_fault(64, fault), "misaligned")

    def test_boundary_access_is_valid(self):
        self.assertEqual(classify_fault(64, Fault("data", 60, 4)), "none")


if __name__ == "__main__":
    unittest.main(verbosity=2)
