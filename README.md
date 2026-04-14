# TitanCore Mod

A DLL mod for War Robots featuring an ImGui overlay menu and Denuvo Anti-Cheat bypass.

## Features

- Bypasses Denuvo Anti-Cheat (stops services, patches handshake, clears logs)
- In-game ImGui menu rendered via DirectX 11 Present hook
- Gameplay patches (noclip, damage boost, rapid fire, and more)

## Building

Run `build.bat` — it will auto-detect your Visual Studio installation and produce `TitanCoreMod.dll`.

## Usage

Inject `TitanCoreMod.dll` into the War Robots process using your preferred injector.

## Keybinds

| Key    | Action      |
|--------|-------------|
| INSERT | Toggle menu |

## Notes

- Requires Visual Studio with C++ tools (any version/edition)
- Built with C++17