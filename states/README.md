# Save states

Expected directory: `Hdd1:/WiiGalaxy360/states/`.

The current host-side state API stores the implemented CPU registers plus MEM1/MEM2 RAM, with a versioned header and integrity hash. It is not yet wired to the Xbox 360 frontend or storage paths, and it does not capture GPU, audio, IOS, FPU, MMU, device registers, or pending interrupts. Do not treat it as a complete Wii-machine snapshot.
