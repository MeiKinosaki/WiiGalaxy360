// Minimal entry point used only to validate the Xenon link stage.
// This is a toolchain smoke test, not the Wii emulator's runtime.
int main() {
    for (;;) {
        // Keep the test binary alive if launched on hardware.
    }
    return 0;
}
