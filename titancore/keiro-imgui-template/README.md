# Keiro + ImGui DX11 Hook Template

A clean DLL injection template using **Dear ImGui** rendered via a **DirectX 11 Present hook** (Keiro-style vtable hook).

---

## Project Structure

```
keiro-imgui-template/
├── src/
│   ├── dllmain.cpp     # DLL entry point, thread management
│   ├── pch.h           # Precompiled header
│   ├── hooks.h/.cpp    # DX11 Present hook (Keiro-style), ImGui init
│   └── menu.h/.cpp     # Your ImGui menu/UI lives here
└── deps/
    ├── imgui/          # Dear ImGui source files (see below)
    └── minhook/        # MinHook header + lib (see below)
```

---

## Dependencies — What You Need to Add

### 1. Dear ImGui
Download from: https://github.com/ocornut/imgui

Copy these files into `deps/imgui/`:
- `imgui.h` / `imgui.cpp`
- `imgui_draw.cpp`
- `imgui_tables.cpp`
- `imgui_widgets.cpp`
- `imgui_internal.h`
- `imconfig.h`
- `imgui_impl_dx11.h` / `imgui_impl_dx11.cpp`  (from `backends/`)
- `imgui_impl_win32.h` / `imgui_impl_win32.cpp` (from `backends/`)

### 2. MinHook
Download from: https://github.com/TsudaKageyu/minhook

Copy into `deps/minhook/`:
- `MinHook.h`
- `MinHook.lib` (pre-built, or build it yourself)

---

## Building (Visual Studio)

1. Create a new **DLL** project in Visual Studio (C++, x64).
2. Add all files from `src/` and the ImGui `.cpp` files from `deps/imgui/` to the project.
3. Link `MinHook.lib` and `d3d11.lib`.
4. Set **Configuration Type** to `Dynamic Library (.dll)`.
5. Build in **Release x64** (or Debug for testing).

---

## How It Works

1. `DllMain` spawns a thread running `MainThread`.
2. `Hooks::Init()` creates a dummy DX11 device/swapchain, reads the **vtable** to get the real `Present` function pointer, then hooks it with MinHook.
3. On the first call to the hooked `Present`, ImGui is initialized against the game's real device and swap chain.
4. Every frame, ImGui renders `Menu::Render()` before calling the original `Present`.
5. Press **INSERT** to toggle the menu, **END** to unload the DLL.

---

## Customizing the Menu

Edit `src/menu.cpp` — add your own features inside `Menu::Render()`.
All standard ImGui widgets work: checkboxes, sliders, buttons, tabs, etc.

---

## Keybinds

| Key    | Action           |
|--------|------------------|
| INSERT | Toggle menu      |
| END    | Unload DLL       |
