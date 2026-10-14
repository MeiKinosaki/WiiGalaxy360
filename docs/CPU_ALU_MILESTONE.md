# CPU ALU milestone: initial C implementation

## Included

- `ppc_subfc`: computes RB - RA and sets XER CA when there is no unsigned borrow.
- `ppc_addc`: computes 32-bit addition and sets XER CA from the carry-out.
- Signed and unsigned compare helpers.
- AND, OR and XOR helpers.
- Host-side test target: `make test`.

## Limits

This is a small arithmetic component, not a full PowerPC/Broadway interpreter. It does not decode opcodes, manage GPRs or condition registers, implement exceptions, or integrate with the memory bus. It has not been compiled or run in the Xbox 360 toolchain. Do not claim tests passed until the command has actually run in a checkout.
