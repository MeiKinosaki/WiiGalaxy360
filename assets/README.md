# Startup assets

Optional configured paths:
- `Hdd1:/WiiGalaxy360/assets/wii_intro.png`
- `Hdd1:/WiiGalaxy360/assets/wii_intro_audio.ogg`

The startup image/audio are not bundled and are not yet rendered/played by a native Xbox 360 runtime. Supply artwork and audio you have the right to use. The intended startup sequence is: display image, play the selected audio once when enabled, then open the game library; controller input should allow skipping the intro. The final native implementation must support missing files and audio-disabled mode without blocking startup.
