# CPU step milestone

This milestone adds a small host-side PowerPC CPU state and instruction-step interface.

## Implemented

- 32 general-purpose 32-bit registers, PC, LR, CTR, CR, and the ALU carry state.
- Reset routine that initializes this state and accepts an entry PC.
- Instruction decode and execution for the X-form `subfc` instruction (primary opcode 31, XO 8).
- Host tests for reset, subtraction result, carry/no-borrow behavior, PC advancement, and rejection of an unsupported instruction.
- GitHub Actions workflow to compile and run `make test` on pushes to `main`, pull requests, and manual dispatch.

## Limitations

This is not a complete Broadway CPU, Wii emulator, or Xbox 360 executable. The step interface currently implements only `subfc`; unsupported instructions return `PPC_STEP_UNSUPPORTED` without advancing PC. Condition-register updates, exception delivery, memory access, branch/delay behavior, floating point, paired singles, MMU, GPU, audio, IOS, and hardware integration remain unimplemented. The CI workflow has been committed, but its pass/fail status must be read from the actual GitHub Actions run before tests are described as passing.
