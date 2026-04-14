#include "pch.h"
#include "bypass.h"
#include <winsvc.h>
#pragma comment(lib, "advapi32.lib")

// -------------------------------------------------------
// Service Control Manager helpers
// -------------------------------------------------------

static bool StopNamedService(const char* name)
{
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) return false;

    SC_HANDLE hSvc = OpenServiceA(hSCM, name, SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!hSvc)
    {
        CloseServiceHandle(hSCM);
        return false;
    }

    SERVICE_STATUS ss{};
    ControlService(hSvc, SERVICE_CONTROL_STOP, &ss);
    CloseServiceHandle(hSvc);
    CloseServiceHandle(hSCM);
    return true;
}

static bool IsServiceStopped(const char* name)
{
    SC_HANDLE hSCM = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) return true;   // can't open SCM -> assume gone

    SC_HANDLE hSvc = OpenServiceA(hSCM, name, SERVICE_QUERY_STATUS);
    if (!hSvc)
    {
        CloseServiceHandle(hSCM);
        return true;           // service not installed -> effectively stopped
    }

    SERVICE_STATUS_PROCESS sp{};
    DWORD needed = 0;
    QueryServiceStatusEx(hSvc, SC_STATUS_PROCESS_INFO,
        reinterpret_cast<LPBYTE>(&sp), sizeof(sp), &needed);

    CloseServiceHandle(hSvc);
    CloseServiceHandle(hSCM);
    return sp.dwCurrentState == SERVICE_STOPPED;
}

// Stops both Denuvo services, retrying up to maxRetries times with 2s pauses.
static void StopDenuvoServices(int maxRetries = 5)
{
    for (int i = 0; i < maxRetries; i++)
    {
        StopNamedService("Denuvo Anti-Cheat");
        StopNamedService("Denuvo Anti-Cheat Update Service");
        Sleep(2000);

        if (IsServiceStopped("Denuvo Anti-Cheat") &&
            IsServiceStopped("Denuvo Anti-Cheat Update Service"))
            return;
    }
}

// -------------------------------------------------------
// Log file cleanup
// -------------------------------------------------------

static void ClearLogs()
{
    const char* logDir = "C:\\Program Files\\Denuvo Anti-Cheat\\Logs";
    std::string pattern = std::string(logDir) + "\\*";

    WIN32_FIND_DATAA fd{};
    HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;

    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::string full = std::string(logDir) + "\\" + fd.cFileName;
            DeleteFileA(full.c_str());
        }
    } while (FindNextFileA(h, &fd));

    FindClose(h);
}

// -------------------------------------------------------
// Bypass::Run
// -------------------------------------------------------

bool Bypass::Run()
{
    // Step 1: wait up to 60 seconds for the runtime DLL to appear in this process
    HMODULE hRuntime = nullptr;
    for (int i = 0; i < 60; i++)
    {
        hRuntime = GetModuleHandleA("denuvo-anti-cheat-runtime.dll");
        if (hRuntime) break;
        Sleep(1000);
    }
    if (!hRuntime) return false;

    // Step 2: call stopServices() — disables the runtime's internal watchdog
    using StopServices_t = void(*)();
    if (auto fn = reinterpret_cast<StopServices_t>(
            GetProcAddress(hRuntime, "stopServices")))
        fn();

    // Step 3: stop the kernel-mode driver via Service Control Manager
    StopDenuvoServices();

    // Step 4: patch runtimeApiDriverHandshakeStatus -> always return 1 (SUCCESS)
    //   mov eax, 1  (B8 01 00 00 00)
    //   ret         (C3)
    LPVOID handshake = nullptr;
    for (int i = 0; i < 60 && !handshake; ++i)
    {
        handshake = reinterpret_cast<LPVOID>(
            GetProcAddress(hRuntime, "runtimeApiDriverHandshakeStatus"));
        if (!handshake)
            Sleep(1000);
    }
    if (handshake)
    {
        BYTE patch[] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 };
        DWORD oldProt = 0;
        if (VirtualProtect(handshake, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProt))
        {
            memcpy(handshake, patch, sizeof(patch));
            FlushInstructionCache(GetCurrentProcess(), handshake, sizeof(patch));
            VirtualProtect(handshake, sizeof(patch), oldProt, &oldProt);
        }
    }

    // Step 5: remove leftover log files
    ClearLogs();

    return true;
}
