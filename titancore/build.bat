@echo off
echo Building Denuvo Bypass Mod DLL...

:: Use vswhere to find any VS installation (any version, any edition, any drive)
set VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found. Is Visual Studio installed?
    pause
    exit /b 1
)

for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VS_PATH=%%i

if not defined VS_PATH (
    echo ERROR: No Visual Studio with C++ tools found.
    pause
    exit /b 1
)

echo Found Visual Studio at: %VS_PATH%
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

:: -------------------------------------------------------
:: Step 1: Compile MinHook sources -> MinHook.lib
:: -------------------------------------------------------
echo Compiling MinHook...
set MH_INC=/I minhook\minhook-1.3.3\include /I minhook\minhook-1.3.3\src
cl.exe /c /nologo minhook\minhook-1.3.3\src\hook.c %MH_INC%
cl.exe /c /nologo minhook\minhook-1.3.3\src\trampoline.c %MH_INC%
cl.exe /c /nologo minhook\minhook-1.3.3\src\buffer.c %MH_INC%
cl.exe /c /nologo minhook\minhook-1.3.3\src\hde\hde32.c %MH_INC%
cl.exe /c /nologo minhook\minhook-1.3.3\src\hde\hde64.c %MH_INC%
lib.exe /nologo hook.obj trampoline.obj buffer.obj hde32.obj hde64.obj /OUT:MinHook.lib

:: -------------------------------------------------------
:: Step 2: Compile background image resource -> bg.res
:: -------------------------------------------------------
echo Compiling background resource...
rc.exe /nologo /fo bg.res keiro-imgui-template\src\bg_resource.rc

:: -------------------------------------------------------
:: Step 3: Compile the full ImGui mod DLL
:: -------------------------------------------------------
echo Compiling TitanCoreMod.dll...
del /f TitanCoreMod.dll 2>nul

set SRC_DIR=keiro-imgui-template\src
set IMGUI_DIR=external\imgui
set BACKENDS_DIR=external\imgui\backends

set INCLUDES=/I %SRC_DIR% /I %IMGUI_DIR% /I %BACKENDS_DIR% /I minhook\minhook-1.3.3\include

set MOD_SOURCES=^
    %SRC_DIR%\dllmain.cpp ^
    %SRC_DIR%\bypass.cpp ^
    %SRC_DIR%\hooks.cpp ^
    %SRC_DIR%\menu.cpp ^
    %SRC_DIR%\patches.cpp

set IMGUI_SOURCES=^
    %IMGUI_DIR%\imgui.cpp ^
    %IMGUI_DIR%\imgui_draw.cpp ^
    %IMGUI_DIR%\imgui_tables.cpp ^
    %IMGUI_DIR%\imgui_widgets.cpp ^
    %BACKENDS_DIR%\imgui_impl_dx11.cpp ^
    %BACKENDS_DIR%\imgui_impl_win32.cpp

set LIBS=MinHook.lib d3d11.lib user32.lib kernel32.lib advapi32.lib windowscodecs.lib shlwapi.lib ole32.lib

cl.exe /LD /EHsc /std:c++17 /O2 /nologo ^
    %INCLUDES% ^
    %MOD_SOURCES% ^
    %IMGUI_SOURCES% ^
    bg.res ^
    %LIBS% ^
    /Fe:TitanCoreMod.dll

if %ERRORLEVEL% EQU 0 (
    echo.
    echo SUCCESS! TitanCoreMod.dll created!
    del /f *.obj bg.res 2>nul
) else (
    echo.
    echo Build failed.
)

pause