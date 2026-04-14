#include "pch.h"
#include "hooks.h"
#include "menu.h"
#include "patches.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// MinHook
#include "MinHook.h"

// WIC (used for embedded background image decoding)
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#include <ole2.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// -------------------------------------------------------
// Globals
// -------------------------------------------------------
static ID3D11Device*            g_pd3dDevice        = nullptr;
static ID3D11DeviceContext*     g_pd3dContext        = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
static HWND                     g_hWnd              = nullptr;
static WNDPROC                  g_oWndProc          = nullptr;
static bool                     g_ImGuiInitialized  = false;

// Background image texture (loaded from Background.jpg next to the DLL)
ID3D11ShaderResourceView* Hooks::g_pBackgroundSRV = nullptr;
int Hooks::g_BackgroundW = 0;
int Hooks::g_BackgroundH = 0;

// Present typedef  (vtable slot 8)
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
static Present_t o_Present = nullptr;

// -------------------------------------------------------
// Forward declarations
// -------------------------------------------------------
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static void Log(const char* fmt, ...);

// -------------------------------------------------------
// WndProc hook — lets ImGui capture input
// -------------------------------------------------------
LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    return CallWindowProcW(g_oWndProc, hWnd, uMsg, wParam, lParam);
}

// -------------------------------------------------------
// Load Background image from embedded RCDATA resource into a D3D11 SRV
// -------------------------------------------------------
static bool LoadBackgroundTexture(ID3D11Device* pDevice)
{
    // Find the RCDATA resource baked into this DLL (resource ID 1)
    HMODULE hSelf = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&LoadBackgroundTexture, &hSelf);

    HRSRC   hRes  = FindResourceA(hSelf, MAKEINTRESOURCEA(1), RT_RCDATA);
    if (!hRes) return false;
    HGLOBAL hGlob = LoadResource(hSelf, hRes);
    if (!hGlob) return false;
    DWORD   sz    = SizeofResource(hSelf, hRes);
    void*   pData = LockResource(hGlob);
    if (!pData || sz == 0) return false;

    // Wrap the raw bytes in an IStream so WIC can decode them
    IStream* pStream = SHCreateMemStream((const BYTE*)pData, sz);
    if (!pStream) return false;

    bool comInitialized = false;
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
        comInitialized = true;

    IWICImagingFactory* pWic = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWic))))
    {
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }

    IWICBitmapDecoder* pDecoder = nullptr;
    if (FAILED(pWic->CreateDecoderFromStream(pStream, nullptr,
                                             WICDecodeMetadataCacheOnDemand, &pDecoder)))
    {
        pWic->Release();
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }

    IWICBitmapFrameDecode* pFrame = nullptr;
    if (FAILED(pDecoder->GetFrame(0, &pFrame)))
    {
        pDecoder->Release();
        pWic->Release();
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }

    IWICFormatConverter* pConv = nullptr;
    if (FAILED(pWic->CreateFormatConverter(&pConv)) || !pConv)
    {
        pFrame->Release();
        pDecoder->Release();
        pWic->Release();
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }

    if (FAILED(pConv->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA,
                                 WICBitmapDitherTypeNone, nullptr, 0.0f,
                                 WICBitmapPaletteTypeCustom)))
    {
        pConv->Release();
        pFrame->Release();
        pDecoder->Release();
        pWic->Release();
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }

    UINT w = 0, h = 0;
    if (FAILED(pConv->GetSize(&w, &h)))
    {
        pConv->Release();
        pFrame->Release();
        pDecoder->Release();
        pWic->Release();
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }
    std::vector<BYTE> px(w * h * 4);
    if (FAILED(pConv->CopyPixels(nullptr, w * 4, (UINT)px.size(), px.data())))
    {
        pConv->Release();
        pFrame->Release();
        pDecoder->Release();
        pWic->Release();
        pStream->Release();
        if (comInitialized) CoUninitialize();
        return false;
    }

    D3D11_TEXTURE2D_DESC td{};
    td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM; td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = px.data(); initData.SysMemPitch = w * 4;

    ID3D11Texture2D* pTex = nullptr;
    bool ok = false;
    if (SUCCEEDED(pDevice->CreateTexture2D(&td, &initData, &pTex)))
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        ok = SUCCEEDED(pDevice->CreateShaderResourceView(pTex, &srvDesc, &Hooks::g_pBackgroundSRV));
        pTex->Release();
        if (ok) { Hooks::g_BackgroundW = (int)w; Hooks::g_BackgroundH = (int)h; }
    }

    pConv->Release(); pFrame->Release(); pDecoder->Release(); pWic->Release(); pStream->Release();
    if (comInitialized) CoUninitialize();
    return ok;
}

// -------------------------------------------------------
// Helper: create render target from swap chain back buffer
// -------------------------------------------------------
static void CreateRenderTarget(IDXGISwapChain* pSwapChain) {
    ID3D11Texture2D* pBackBuffer = nullptr;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

// -------------------------------------------------------
// Hooked Present
// -------------------------------------------------------
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {

    // --- One-time ImGui init ---
    if (!g_ImGuiInitialized) {
        pSwapChain->GetDevice(IID_PPV_ARGS(&g_pd3dDevice));
        g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

        DXGI_SWAP_CHAIN_DESC sd{};
        pSwapChain->GetDesc(&sd);
        g_hWnd = sd.OutputWindow;

        CreateRenderTarget(pSwapChain);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(g_hWnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

        // Hook WndProc so ImGui gets input
        g_oWndProc = (WNDPROC)SetWindowLongPtrW(g_hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

        g_ImGuiInitialized = true;

        // Load background image (silently ignored if file not found)
        LoadBackgroundTexture(g_pd3dDevice);
    }

    // --- Every frame ---
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Tick gameplay systems before rendering (position writes before present)
    Patches::NoclipTick();
    Patches::GameplayTick();

    // Draw our menu
    Menu::Render();

    ImGui::Render();
    g_pd3dContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return o_Present(pSwapChain, SyncInterval, Flags);
}

// -------------------------------------------------------
// Get Present pointer via dummy swap chain
// -------------------------------------------------------
static void* GetPresentPtr() {
    // Create a temporary hidden window — GetDesktopWindow() is not a valid
    // render target and will cause D3D11CreateDeviceAndSwapChain to fail.
    WNDCLASSEXA wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = DefWindowProcA;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.lpszClassName = "WRModDummyWnd";
    RegisterClassExA(&wc);

    HWND hWnd = CreateWindowExA(0, "WRModDummyWnd", nullptr,
        WS_OVERLAPPEDWINDOW, 0, 0, 8, 8,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!hWnd) {
        Log("[Hooks] CreateWindowExA failed 0x%08X", GetLastError());
        UnregisterClassA("WRModDummyWnd", wc.hInstance);
        return nullptr;
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount        = 1;
    sd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width   = 8;
    sd.BufferDesc.Height  = 8;
    sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count   = 1;
    sd.Windowed           = TRUE;
    sd.OutputWindow       = hWnd;

    IDXGISwapChain*      pSC  = nullptr;
    ID3D11Device*        pDev = nullptr;
    ID3D11DeviceContext* pCtx = nullptr;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &sd, &pSC, &pDev, nullptr, &pCtx);

    Log("[Hooks] D3D11CreateDeviceAndSwapChain hr=0x%08X", hr);

    void* presentPtr = nullptr;
    if (SUCCEEDED(hr) && pSC)
        presentPtr = (*reinterpret_cast<void***>(pSC))[8];

    if (pCtx) pCtx->Release();
    if (pDev) pDev->Release();
    if (pSC)  pSC->Release();

    DestroyWindow(hWnd);
    UnregisterClassA("WRModDummyWnd", wc.hInstance);

    return presentPtr;
}

// -------------------------------------------------------
// Simple log helper (writes next to the DLL in the game dir)
// -------------------------------------------------------
static void Log(const char* fmt, ...)
{
    static bool s_cleared = false;

    char msg[512];
    va_list va;
    va_start(va, fmt);
    _vsnprintf_s(msg, sizeof(msg), _TRUNCATE, fmt, va);
    va_end(va);

    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) *(slash + 1) = '\0';
    strncat_s(path, "WarRobotsMod.log", 16);

    // First write each injection clears the file; subsequent writes append.
    FILE* f = nullptr;
    fopen_s(&f, path, s_cleared ? "a" : "w");
    if (f) { fprintf(f, "%s\n", msg); fclose(f); s_cleared = true; }
}

// -------------------------------------------------------
// Init / Shutdown
// -------------------------------------------------------
void Hooks::Init() {
    Log("[Hooks] Init called");

    MH_STATUS mhStatus = MH_Initialize();
    Log("[Hooks] MH_Initialize = %d", (int)mhStatus);

    void* pPresent = GetPresentPtr();
    Log("[Hooks] GetPresentPtr = %p", pPresent);

    if (pPresent) {
        MH_STATUS ck = MH_CreateHook(pPresent, &hkPresent, reinterpret_cast<void**>(&o_Present));
        MH_STATUS en = MH_EnableHook(pPresent);
        Log("[Hooks] MH_CreateHook = %d  MH_EnableHook = %d", (int)ck, (int)en);
    } else {
        Log("[Hooks] GetPresentPtr returned null — game may be using DX12, not DX11");
    }
}

void Hooks::Shutdown() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (g_ImGuiInitialized) {
        // Restore WndProc
        SetWindowLongPtrW(g_hWnd, GWLP_WNDPROC, (LONG_PTR)g_oWndProc);

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
        if (g_pd3dContext)          { g_pd3dContext->Release();          g_pd3dContext = nullptr; }
        if (g_pd3dDevice)           { g_pd3dDevice->Release();           g_pd3dDevice = nullptr; }
    }
}
