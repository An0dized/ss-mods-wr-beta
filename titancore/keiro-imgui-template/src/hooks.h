#pragma once
#include "pch.h"

namespace Hooks {
    void Init();
    void Shutdown();

    // Background image texture (loaded from Background.jpg next to the DLL)
    extern ID3D11ShaderResourceView* g_pBackgroundSRV;
    extern int g_BackgroundW, g_BackgroundH;
}
