# Native library UI roadmap

Target: a controller-driven cover grid inspired by Xbox 360 Aurora, using `themes/aurora-blue-white.json` and `config/ui_settings.json`.

Required integration milestones:
1. Native Xbox 360 rendering/input loop and focus/navigation.
2. Scan configured ROM directory and persist catalog/favorites.
3. Render local covers with placeholders and compatibility indicators.
4. Add explicit network configuration and cover metadata provider; support wired Ethernet and Wi-Fi only through network APIs available in the target runtime.
5. Add an optional Wii-inspired startup screen and user-provided/licensed audio.
6. Connect menu actions to CPU execution, save/load, and shutdown.

Current state: configuration and catalog schemas only. The HTML mockup and JSON files are not a native Xbox 360 application and do not prove working controller, audio, networking, or game loading.
