#include "pch.h"
#include "menu.h"
#include "hooks.h"
#include "patches.h"
#include "imgui.h"
#include <cmath>

// -------------------------------------------------------
// Patch toggle states
// -------------------------------------------------------
static bool bPacifyAI      = false;
static bool bNoclip        = false;
static int  iFPSOverride   = 0;
static bool bUseCameraDir = false;

// Weapons
static bool  bRapidFire      = false;
static bool  bAutoFire       = false;
static bool  bInfiniteAmmo   = false;
static bool  bHugeExplosions = false;
static bool  bDamageBoost    = false;
static float fDamageBoostMul = 10.0f;
static bool  bAutoGotoEnemy   = false;
static bool  bGhostGotoEnemy  = false;
static bool  bInvisible       = false;
static bool  bIgnoreAbsorber  = false;
static bool  bIgnorePhaseShift = false;
static bool  bIgnoreCounterShield = false;
static bool  bAntiStealth = false;
static bool  bAutoClaimRewards = false;
static bool  bPhoenixKillStreak = false;

// Abilities / Module
static bool bNoLockedStates = false;
static bool bInst1CD      = false;
static bool bInst2CD      = false;
static bool bInstModCD    = false;
static bool bAbility1Dur  = false;
static bool bAbility2Dur  = false;
static bool bModuleDur    = false;
static bool bAutoAbility1 = false;
static bool bAutoAbility2 = false;
static bool bAutoModule   = false;
// Player features
static bool  bHidePlayerName = false;
static bool  bForceGrounded  = false;
static float fPlayerScale    = 0.0f;
static float fCameraFOV      = 0.0f;
static float fCameraZoom     = 0.0f;
static float fJumpPower      = 100.0f;
static float fBoostPower     = 100.0f;

// -------------------------------------------------------
// Keybinds
// -------------------------------------------------------
enum {
    BIND_PACIFY_AI = 0, BIND_HIDE_NAME,   BIND_NOCLIP,       BIND_AUTO_ENEMY,
    BIND_RAPID_FIRE,    BIND_AUTO_FIRE,   BIND_INF_AMMO,     BIND_HUGE_EXP,
    BIND_NO_LOCKED,     BIND_INST1CD,     BIND_INST2CD,      BIND_INSTMODCD,
    BIND_ABILITY1DUR,   BIND_ABILITY2DUR, BIND_MODULEDUR,
    BIND_AUTO_AB1,      BIND_AUTO_AB2,    BIND_AUTO_MOD,
    BIND_JUMP,
    BIND_BOOST_FORWARD,
    BIND_COUNT
};
static int    g_Binds[BIND_COUNT] = {};  // VK code, 0 = unbound
static int    g_CapturingBind     = -1;  // index being captured, -1 = none
static int    g_CapturingNcBind   = -1;  // noclip move bind being captured, -1 = none
static UINT64 g_CaptureStartTick  = 0;

// -------------------------------------------------------
// Preferences  (saved/loaded from WarRobotsMod.ini)
// -------------------------------------------------------
static void GetPrefsPath(char path[MAX_PATH])
{
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    char* slash = strrchr(path, '\\'); if (slash) *(slash + 1) = '\0';
    strncat_s(path, MAX_PATH, "WarRobotsMod.ini", 16);
}
static bool PrefGetBool (const char* p, const char* k, bool  d) { return GetPrivateProfileIntA("Prefs", k, d ? 1 : 0, p) != 0; }
static int   PrefGetInt  (const char* p, const char* k, int   d) { return (int)GetPrivateProfileIntA("Prefs", k, d, p); }
static float PrefGetFloat(const char* p, const char* k, float d) {
    char buf[32], def[32]; _snprintf_s(def, sizeof(def), _TRUNCATE, "%.6f", d);
    GetPrivateProfileStringA("Prefs", k, def, buf, sizeof(buf), p);
    return (float)atof(buf);
}
static void PrefSetBool (const char* p, const char* k, bool  v) { WritePrivateProfileStringA("Prefs", k, v ? "1" : "0", p); }
static void PrefSetInt  (const char* p, const char* k, int   v) { char buf[16]; _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d", v); WritePrivateProfileStringA("Prefs", k, buf, p); }
static void PrefSetFloat(const char* p, const char* k, float v) {
    char buf[32]; _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%.6f", v); WritePrivateProfileStringA("Prefs", k, buf, p);
}

static void SavePrefs()
{
    char p[MAX_PATH]; GetPrefsPath(p);
    PrefSetBool (p, "PacifyAI",      bPacifyAI);
    PrefSetInt  (p, "FPSOverride",    iFPSOverride);
    PrefSetBool (p, "UseCamera",      bUseCameraDir);
    PrefSetBool (p, "RapidFire",  bRapidFire);
    PrefSetBool (p, "AutoFire",   bAutoFire);
    PrefSetBool (p, "InfAmmo",    bInfiniteAmmo);
    PrefSetBool (p, "HugeExp",    bHugeExplosions);
    PrefSetBool (p, "DmgBoost",   bDamageBoost);
    PrefSetFloat(p, "DmgBoostMul",fDamageBoostMul);
    PrefSetBool (p, "AutoEnemy",  bAutoGotoEnemy);
    PrefSetBool (p, "GhostEnemy", bGhostGotoEnemy);
    PrefSetBool (p, "Invisible",  bInvisible);
    PrefSetBool (p, "IgnoreAbsorber", bIgnoreAbsorber);
    PrefSetBool (p, "IgnorePhaseShift", bIgnorePhaseShift);
    PrefSetBool (p, "IgnoreCounterShield", bIgnoreCounterShield);
    PrefSetBool (p, "AntiStealth", bAntiStealth);
    PrefSetBool (p, "AutoClaimRewards", bAutoClaimRewards);
    PrefSetBool (p, "PhoenixKillStreak", bPhoenixKillStreak);
    PrefSetFloat(p, "GotoOffX",   Patches::g_GotoEnemyOffsetX);
    PrefSetFloat(p, "GotoOffY",   Patches::g_GotoEnemyOffsetY);
    PrefSetFloat(p, "GotoOffZ",   Patches::g_GotoEnemyOffsetZ);
    PrefSetBool (p, "NoLocks",    bNoLockedStates);
    PrefSetBool (p, "Inst1CD",    bInst1CD);
    PrefSetBool (p, "Inst2CD",    bInst2CD);
    PrefSetBool (p, "InstModCD",  bInstModCD);
    PrefSetBool (p, "Ab1Dur",     bAbility1Dur);
    PrefSetBool (p, "Ab2Dur",     bAbility2Dur);
    PrefSetBool (p, "ModDur",     bModuleDur);
    PrefSetBool (p, "AutoAb1",    bAutoAbility1);
    PrefSetBool (p, "AutoAb2",    bAutoAbility2);
    PrefSetBool (p, "AutoMod",    bAutoModule);
    PrefSetFloat(p, "NoclipSpeed",  Patches::g_NoclipSpeed);
    PrefSetFloat(p, "Ab1DurVal",    Patches::g_Ability1DurVal);
    PrefSetFloat(p, "Ab2DurVal",    Patches::g_Ability2DurVal);
    PrefSetFloat(p, "ModDurVal",    Patches::g_ModuleDurVal);
    PrefSetFloat(p, "AutoA1Int",    Patches::g_AutoA1Interval);
    PrefSetFloat(p, "AutoA2Int",    Patches::g_AutoA2Interval);
    PrefSetFloat(p, "AutoModInt",   Patches::g_AutoModInterval);
    PrefSetBool (p, "HideName",     bHidePlayerName);
    PrefSetBool (p, "ForceGrounded", bForceGrounded);
    PrefSetFloat(p, "PlayerScale",  fPlayerScale);
    PrefSetFloat(p, "CameraFOV",    fCameraFOV);
    PrefSetFloat(p, "CameraZoom",   fCameraZoom);
    PrefSetFloat(p, "JumpPower",    fJumpPower);
    PrefSetFloat(p, "BoostPower",   fBoostPower);
    for (int i = 0; i < BIND_COUNT; i++)
    {
        char key[16]; _snprintf_s(key, sizeof(key), _TRUNCATE, "Bind_%d", i);
        PrefSetInt(p, key, g_Binds[i]);
    }
    PrefSetInt(p, "NcFwd",    Patches::g_NcKeyFwd);
    PrefSetInt(p, "NcBack",   Patches::g_NcKeyBack);
    PrefSetInt(p, "NcRight",  Patches::g_NcKeyRight);
    PrefSetInt(p, "NcLeft",   Patches::g_NcKeyLeft);
    PrefSetInt(p, "NcUp",     Patches::g_NcKeyUp);
    PrefSetInt(p, "NcDown",   Patches::g_NcKeyDown);
    PrefSetInt(p, "NcSprint", Patches::g_NcKeySprint);
}

static void LoadPrefs()
{
    char p[MAX_PATH]; GetPrefsPath(p);
    bPacifyAI      = PrefGetBool(p, "PacifyAI",      false);
    iFPSOverride   = PrefGetInt (p, "FPSOverride",    0);
    bUseCameraDir  = PrefGetBool(p, "UseCamera",      false);
    bRapidFire    = PrefGetBool (p, "RapidFire",  false);
    bAutoFire     = PrefGetBool (p, "AutoFire",   false);
    bInfiniteAmmo = PrefGetBool (p, "InfAmmo",    false);
    bHugeExplosions = PrefGetBool(p, "HugeExp",   false);
    bDamageBoost    = PrefGetBool (p, "DmgBoost",    false);
    fDamageBoostMul = PrefGetFloat(p, "DmgBoostMul", 10.0f);
    bAutoGotoEnemy  = PrefGetBool(p, "AutoEnemy",   false);
    bGhostGotoEnemy = PrefGetBool(p, "GhostEnemy",  false);
    bInvisible      = PrefGetBool(p, "Invisible",   false);
    bIgnoreAbsorber = PrefGetBool(p, "IgnoreAbsorber", false);
    bIgnorePhaseShift = PrefGetBool(p, "IgnorePhaseShift", false);
    bIgnoreCounterShield = PrefGetBool(p, "IgnoreCounterShield", false);
    bAntiStealth = PrefGetBool(p, "AntiStealth", false);
    bAutoClaimRewards = PrefGetBool(p, "AutoClaimRewards", false);
    bPhoenixKillStreak = PrefGetBool(p, "PhoenixKillStreak", false);
    Patches::g_GotoEnemyOffsetX = PrefGetFloat(p, "GotoOffX", 0.0f);
    Patches::g_GotoEnemyOffsetY = PrefGetFloat(p, "GotoOffY", 0.0f);
    Patches::g_GotoEnemyOffsetZ = PrefGetFloat(p, "GotoOffZ", 0.0f);
    bNoLockedStates = PrefGetBool(p, "NoLocks",   false);
    bInst1CD      = PrefGetBool (p, "Inst1CD",    false);
    bInst2CD      = PrefGetBool (p, "Inst2CD",    false);
    bInstModCD    = PrefGetBool (p, "InstModCD",  false);
    bAbility1Dur  = PrefGetBool (p, "Ab1Dur",     false);
    bAbility2Dur  = PrefGetBool (p, "Ab2Dur",     false);
    bModuleDur    = PrefGetBool (p, "ModDur",     false);
    bAutoAbility1 = PrefGetBool (p, "AutoAb1",    false);
    bAutoAbility2 = PrefGetBool (p, "AutoAb2",    false);
    bAutoModule   = PrefGetBool (p, "AutoMod",    false);

    Patches::g_NoclipSpeed     = PrefGetFloat(p, "NoclipSpeed",  800.0f);
    Patches::g_Ability1DurVal  = PrefGetFloat(p, "Ab1DurVal",    9999.0f);
    Patches::g_Ability2DurVal  = PrefGetFloat(p, "Ab2DurVal",    9999.0f);
    Patches::g_ModuleDurVal    = PrefGetFloat(p, "ModDurVal",    9999.0f);
    Patches::g_AutoA1Interval  = PrefGetFloat(p, "AutoA1Int",    1.0f);
    Patches::g_AutoA2Interval  = PrefGetFloat(p, "AutoA2Int",    1.0f);
    Patches::g_AutoModInterval = PrefGetFloat(p, "AutoModInt",   1.0f);
    bHidePlayerName = PrefGetBool (p, "HideName",     false);
    bForceGrounded  = PrefGetBool (p, "ForceGrounded", false);
    fPlayerScale    = PrefGetFloat(p, "PlayerScale",  0.0f);
    fCameraFOV      = PrefGetFloat(p, "CameraFOV",    0.0f);
    fCameraZoom     = PrefGetFloat(p, "CameraZoom",   0.0f);
    fJumpPower      = PrefGetFloat(p, "JumpPower",   100.0f);
    fBoostPower     = PrefGetFloat(p, "BoostPower",  100.0f);
    Patches::g_JumpPower  = fJumpPower;
    Patches::g_BoostPower = fBoostPower;
    for (int i = 0; i < BIND_COUNT; i++)
    {
        char key[16]; _snprintf_s(key, sizeof(key), _TRUNCATE, "Bind_%d", i);
        int def = (i == BIND_NOCLIP) ? 0xBF : 0; // noclip defaults to '/'
        g_Binds[i] = PrefGetInt(p, key, def);
    }
    Patches::g_NcKeyFwd    = PrefGetInt(p, "NcFwd",    'W');
    Patches::g_NcKeyBack   = PrefGetInt(p, "NcBack",   'S');
    Patches::g_NcKeyRight  = PrefGetInt(p, "NcRight",  'D');
    Patches::g_NcKeyLeft   = PrefGetInt(p, "NcLeft",   'A');
    Patches::g_NcKeyUp     = PrefGetInt(p, "NcUp",     'E');
    Patches::g_NcKeyDown   = PrefGetInt(p, "NcDown",   'Q');
    Patches::g_NcKeySprint = PrefGetInt(p, "NcSprint", VK_SHIFT);

    // Apply loaded toggle states
    if (bPacifyAI)       Patches::PacifyAI(true);
    Patches::g_FPSOverride   = iFPSOverride;
    if (bNoLockedStates) Patches::NoLockedStates(true);
    if (bHugeExplosions)   Patches::HugeExplosions(true);
    Patches::g_DamageBoost    = bDamageBoost;
    Patches::g_DamageBoostMul = fDamageBoostMul;
    if (bDamageBoost) Patches::DamageBoostHook(true);
    Patches::g_UseCameraDir  = bUseCameraDir;
    Patches::g_RapidFire     = bRapidFire;
    Patches::g_AutoFire      = bAutoFire;
    Patches::g_InfiniteAmmo  = bInfiniteAmmo;
    Patches::g_HugeExplosions = bHugeExplosions;
    Patches::g_AutoGotoEnemy   = bAutoGotoEnemy;
    Patches::g_GhostGotoEnemy  = bGhostGotoEnemy;
    if (bGhostGotoEnemy) Patches::GhostGotoEnemyHook(true);
    Patches::g_Invisible       = bInvisible;
    if (bInvisible) Patches::InvisibleHook(true);
    Patches::g_IgnoreAbsorber = bIgnoreAbsorber;
    Patches::g_IgnorePhaseShift = bIgnorePhaseShift;
    Patches::g_IgnoreCounterShield = bIgnoreCounterShield;
    Patches::g_AntiStealth = bAntiStealth;
    if (bAntiStealth) Patches::AntiEffectHook(true);
    Patches::g_AutoClaimRewards = bAutoClaimRewards;
    if (bPhoenixKillStreak) Patches::KillStreakTextHook(true);
    Patches::g_Instant1CD    = bInst1CD;
    Patches::g_Instant2CD    = bInst2CD;
    Patches::g_InstantModCD  = bInstModCD;
    Patches::g_Ability1Dur   = bAbility1Dur;
    Patches::g_Ability2Dur   = bAbility2Dur;
    Patches::g_ModuleDur     = bModuleDur;
    Patches::g_AutoAbility1  = bAutoAbility1;
    Patches::g_AutoAbility2  = bAutoAbility2;
    Patches::g_AutoModule    = bAutoModule;
    Patches::g_PlayerScaleSlider = fPlayerScale;
    Patches::g_CameraFOV         = fCameraFOV;
    Patches::g_CameraZoom        = fCameraZoom;
    Patches::g_ForceGrounded     = bForceGrounded;
    if (bForceGrounded) Patches::ForceGroundedHook(true);
    if (bHidePlayerName) Patches::HidePlayerName(true);
}

// -------------------------------------------------------
// Keybind helpers
// -------------------------------------------------------
static const char* VKeyName(int vk, char* buf, int bufsz)
{
    if (vk == 0) { strncpy_s(buf, bufsz, "Unbound", _TRUNCATE); return buf; }
    UINT sc = MapVirtualKeyA((UINT)vk, MAPVK_VK_TO_VSC);
    if (sc)
    {
        LONG lp = (LONG)(sc << 16);
        if (vk == VK_INSERT   || vk == VK_DELETE  || vk == VK_HOME  ||
            vk == VK_END      || vk == VK_PRIOR    || vk == VK_NEXT  ||
            vk == VK_UP       || vk == VK_DOWN     || vk == VK_LEFT  || vk == VK_RIGHT ||
            vk == VK_NUMLOCK  || vk == VK_RCONTROL || vk == VK_RMENU || vk == VK_DIVIDE)
            lp |= (1 << 24);
        if (GetKeyNameTextA(lp, buf, bufsz) > 0) return buf;
    }
    _snprintf_s(buf, bufsz, _TRUNCATE, "VK 0x%02X", vk);
    return buf;
}

static void ToggleBoundFeature(int idx)
{
    switch (idx)
    {
    case BIND_PACIFY_AI:   bPacifyAI       = !bPacifyAI;       Patches::PacifyAI(bPacifyAI);                           break;
    case BIND_HIDE_NAME:   bHidePlayerName = !bHidePlayerName; Patches::HidePlayerName(bHidePlayerName);               break;
    case BIND_NOCLIP:      bNoclip         = !bNoclip;         Patches::NoclipEnable(bNoclip);                         break;
    case BIND_AUTO_ENEMY:  bAutoGotoEnemy  = !bAutoGotoEnemy;  Patches::g_AutoGotoEnemy  = bAutoGotoEnemy;             break;
    case BIND_RAPID_FIRE:  bRapidFire      = !bRapidFire;      Patches::g_RapidFire      = bRapidFire;                 break;
    case BIND_AUTO_FIRE:   bAutoFire       = !bAutoFire;       Patches::g_AutoFire       = bAutoFire;                  break;
    case BIND_INF_AMMO:    bInfiniteAmmo   = !bInfiniteAmmo;   Patches::g_InfiniteAmmo   = bInfiniteAmmo;              break;
    case BIND_HUGE_EXP:    bHugeExplosions = !bHugeExplosions; Patches::g_HugeExplosions = bHugeExplosions;
                           Patches::HugeExplosions(bHugeExplosions);                                                    break;
    case BIND_NO_LOCKED:   bNoLockedStates = !bNoLockedStates; Patches::NoLockedStates(bNoLockedStates);               break;
    case BIND_INST1CD:     bInst1CD        = !bInst1CD;        Patches::g_Instant1CD     = bInst1CD;                   break;
    case BIND_INST2CD:     bInst2CD        = !bInst2CD;        Patches::g_Instant2CD     = bInst2CD;                   break;
    case BIND_INSTMODCD:   bInstModCD      = !bInstModCD;      Patches::g_InstantModCD   = bInstModCD;                 break;
    case BIND_ABILITY1DUR: bAbility1Dur    = !bAbility1Dur;    Patches::g_Ability1Dur    = bAbility1Dur;               break;
    case BIND_ABILITY2DUR: bAbility2Dur    = !bAbility2Dur;    Patches::g_Ability2Dur    = bAbility2Dur;               break;
    case BIND_MODULEDUR:   bModuleDur      = !bModuleDur;      Patches::g_ModuleDur      = bModuleDur;                 break;
    case BIND_AUTO_AB1:    bAutoAbility1   = !bAutoAbility1;   Patches::g_AutoAbility1   = bAutoAbility1;              break;
    case BIND_AUTO_AB2:    bAutoAbility2   = !bAutoAbility2;   Patches::g_AutoAbility2   = bAutoAbility2;              break;
    case BIND_AUTO_MOD:    bAutoModule = !bAutoModule; Patches::g_AutoModule = bAutoModule;                    break;
    case BIND_JUMP:        Patches::JumpUp();                                                      break;
    case BIND_BOOST_FORWARD: Patches::BoostForward();                                           break;
    }
    SavePrefs();
}

static void DrawBindRow(const char* label, int idx)
{
    // Default binds: noclip = '/', everything else = unbound (0)
    static const int k_BindDefaults[BIND_COUNT] = {
        0,     // BIND_PACIFY_AI
        0,     // BIND_HIDE_NAME
        0xBF,  // BIND_NOCLIP  ('/')
        0,     // BIND_AUTO_ENEMY
        0, 0, 0, 0,   // BIND_RAPID_FIRE .. BIND_HUGE_EXP
        0, 0, 0, 0,   // BIND_NO_LOCKED .. BIND_INSTMODCD
        0, 0, 0,      // BIND_ABILITY1DUR .. BIND_MODULEDUR
        0, 0, 0,      // BIND_AUTO_AB1 .. BIND_AUTO_MOD
        0,            // BIND_JUMP
        0,            // BIND_BOOST_FORWARD
    };

    ImGui::PushID(idx);
    ImGui::TextUnformatted(label);
    float fullWidth = ImGui::GetContentRegionAvail().x;
    bool nonDefault = (g_Binds[idx] != k_BindDefaults[idx]);
    if (nonDefault && g_CapturingBind != idx)
    {
        ImGui::SameLine(fullWidth - 164.0f);
        if (ImGui::Button("Reset", ImVec2(50, 0)))
        {
            g_Binds[idx] = k_BindDefaults[idx];
            SavePrefs();
        }
    }
    ImGui::SameLine(fullWidth - 110.0f);
    if (g_CapturingBind == idx)
    {
        ImGui::Button("[ Press key... ]", ImVec2(110, 0));
    }
    else
    {
        char buf[64];
        VKeyName(g_Binds[idx], buf, sizeof(buf));
        if (ImGui::Button(buf, ImVec2(110, 0)))
        {
            g_CapturingBind    = idx;
            g_CaptureStartTick = GetTickCount64();
        }
    }
    ImGui::PopID();
}

// DrawBindRow for noclip movement keys (writes directly to Patches::g_NcKeyXxx)
static void DrawNcBindRow(const char* label, int* pVK, int captureId)
{
    static const int k_NcDefaults[7] = { 'W', 'S', 'D', 'A', 'E', 'Q', VK_SHIFT };

    ImGui::PushID(captureId + 1000);
    ImGui::TextUnformatted(label);
    float fullWidth = ImGui::GetContentRegionAvail().x;
    bool nonDefault = (*pVK != k_NcDefaults[captureId]);
    if (nonDefault && g_CapturingNcBind != captureId)
    {
        ImGui::SameLine(fullWidth - 164.0f);
        if (ImGui::Button("Reset", ImVec2(50, 0)))
        {
            *pVK = k_NcDefaults[captureId];
            SavePrefs();
        }
    }
    ImGui::SameLine(fullWidth - 110.0f);
    if (g_CapturingNcBind == captureId)
    {
        ImGui::Button("[ Press key... ]", ImVec2(110, 0));
    }
    else
    {
        char buf[64];
        VKeyName(*pVK, buf, sizeof(buf));
        if (ImGui::Button(buf, ImVec2(110, 0)))
        {
            g_CapturingNcBind  = captureId;
            g_CaptureStartTick = GetTickCount64();
        }
    }
    ImGui::PopID();
}
// -------------------------------------------------------
static bool ToggleRow(const char* label, const char* desc, bool& val)
{
    bool changed = false;
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    if (desc && desc[0]) { ImGui::SameLine(); ImGui::TextDisabled("  %s", desc); }
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);
    changed = ImGui::Checkbox("##toggle", &val);
    ImGui::PopID();
    return changed;
}

// -------------------------------------------------------
// Helper: toggle + right-aligned float input on same row
// Returns true if checkbox changed.
// -------------------------------------------------------
static bool DurRow(const char* id, const char* label, bool& val, float& dur)
{
    bool changed = false;
    ImGui::PushID(id);
    changed = ImGui::Checkbox("##chk", &val);
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 85.0f);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::InputFloat("##dur", &dur, 0.1f, 1.0f, "%.2f");
    if (dur < 0.001f) dur = 0.001f;
    ImGui::PopID();
    return changed;
}

// -------------------------------------------------------
// Helper: toggle + right-aligned interval input on same row
// Returns true if checkbox changed.
// -------------------------------------------------------
static bool AutoRow(const char* id, const char* label, bool& val, float& interval)
{
    bool changed = false;
    ImGui::PushID(id);
    changed = ImGui::Checkbox("##chk", &val);
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 85.0f);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::InputFloat("##iv", &interval, 0.01f, 0.1f, "%.2fs");
    if (interval < 0.0f) interval = 0.0f;
    ImGui::PopID();
    return changed;
}

// -------------------------------------------------------
// Key handling — INSERT toggles menu, keybinds toggle features
// -------------------------------------------------------
static void HandleKeys()
{
    // INSERT — always process menu toggle
    static bool bInsertPrev = false;
    bool bInsertNow = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
    if (bInsertNow && !bInsertPrev)
    {
        Menu::IsOpen = !Menu::IsOpen;
        if (!Menu::IsOpen) { g_CapturingBind = -1; g_CapturingNcBind = -1; }
    }
    bInsertPrev = bInsertNow;

    // Capture mode for feature toggle binds
    if (g_CapturingBind >= 0)
    {
        if (GetTickCount64() - g_CaptureStartTick >= 150)
        {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                g_Binds[g_CapturingBind] = 0;
                g_CapturingBind = -1;
                SavePrefs();
            }
            else
            {
                for (int vk = 1; vk <= 254; vk++)
                {
                    if (vk == VK_ESCAPE || vk == VK_INSERT || vk == VK_END) continue;
                    if (GetAsyncKeyState(vk) & 0x8000)
                    {
                        g_Binds[g_CapturingBind] = vk;
                        g_CapturingBind = -1;
                        SavePrefs();
                        break;
                    }
                }
            }
        }
        return;
    }

    // Capture mode for noclip movement binds
    if (g_CapturingNcBind >= 0)
    {
        if (GetTickCount64() - g_CaptureStartTick >= 150)
        {
            int* targets[] = { &Patches::g_NcKeyFwd, &Patches::g_NcKeyBack,
                               &Patches::g_NcKeyRight, &Patches::g_NcKeyLeft,
                               &Patches::g_NcKeyUp, &Patches::g_NcKeyDown,
                               &Patches::g_NcKeySprint };
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                *targets[g_CapturingNcBind] = 0;
                g_CapturingNcBind = -1;
                SavePrefs();
            }
            else
            {
                for (int vk = 1; vk <= 254; vk++)
                {
                    if (vk == VK_ESCAPE || vk == VK_INSERT || vk == VK_END) continue;
                    if (GetAsyncKeyState(vk) & 0x8000)
                    {
                        *targets[g_CapturingNcBind] = vk;
                        g_CapturingNcBind = -1;
                        SavePrefs();
                        break;
                    }
                }
            }
        }
        return;
    }

    // Toggle features on key-down edge
    static bool s_prev[256] = {};
    bool s_cur[256] = {};
    for (int vk = 1; vk <= 254; vk++)
        s_cur[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
    for (int i = 0; i < BIND_COUNT; i++)
    {
        int vk = g_Binds[i];
        if (vk <= 0 || vk > 254) continue;
        if (s_cur[vk] && !s_prev[vk])
            ToggleBoundFeature(i);
    }
    memcpy(s_prev, s_cur, sizeof(s_prev));
}

// -------------------------------------------------------
// Main render function called every frame from hkPresent
// -------------------------------------------------------
void Menu::Render() {
    HandleKeys();

    // Load prefs on the first frame
    static bool s_prefsLoaded = false;
    if (!s_prefsLoaded) { s_prefsLoaded = true; LoadPrefs(); }

    // Save prefs when the menu is closed
    static bool s_wasOpen = false;
    if (s_wasOpen && !IsOpen) SavePrefs();
    s_wasOpen = IsOpen;

    if (!IsOpen)
        return;

    static bool  s_minimized   = false;
    static bool  s_prevMin      = false;
    static float s_savedH       = 370.0f;
    float titleH = ImGui::GetFrameHeight();
    ImGuiCond sizeCond;
    float wantH;
    if (s_minimized) {
        wantH    = titleH;
        sizeCond = ImGuiCond_Always;
    } else if (s_prevMin) {
        // Just restored — force saved height this frame
        wantH    = s_savedH;
        sizeCond = ImGuiCond_Always;
    } else {
        wantH    = s_savedH;
        sizeCond = ImGuiCond_FirstUseEver;
    }
    ImGui::SetNextWindowSize(ImVec2(460, wantH), sizeCond);
    ImGui::PushStyleColor(ImGuiCol_TitleBg,          IM_COL32(0, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,    IM_COL32(0, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, IM_COL32(0, 0, 0, 255));
    ImGui::Begin("##TitanCore", nullptr, ImGuiWindowFlags_NoCollapse | (s_minimized ? ImGuiWindowFlags_NoResize : 0));
    ImGui::PopStyleColor(3);
    if (!s_minimized) s_savedH = ImGui::GetWindowSize().y;
    s_prevMin = s_minimized;

    // Rainbow gradient title drawn over the (empty) title bar
    {
        ImDrawList* draw_list  = ImGui::GetForegroundDrawList();
        ImVec2      wPos       = ImGui::GetWindowPos();
        ImVec2      wSz        = ImGui::GetWindowSize();
        ImFont*     font       = ImGui::GetFont();
        float       font_size  = ImGui::GetFontSize();
        const char* title      = "TitanCore";
        ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, title);
        float title_x = wPos.x + (wSz.x - text_size.x) * 0.5f;
        float title_y = wPos.y + 4.0f;

        float time_sec = (float)(GetTickCount64() % 6000) / 1000.0f;
        float phase    = fmodf(time_sec / 6.0f, 1.0f);

        ImVec4 stops[5] = {
            ImVec4(1.0f,   0.0f,   0.502f, 1.0f),
            ImVec4(1.0f,   0.549f, 0.0f,   1.0f),
            ImVec4(0.251f, 0.878f, 0.816f, 1.0f),
            ImVec4(0.541f, 0.169f, 0.886f, 1.0f),
            ImVec4(1.0f,   0.0f,   0.565f, 1.0f)
        };
        auto lerp_color = [](const ImVec4& a, const ImVec4& b, float t) -> ImVec4 {
            return ImVec4(a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t, 1.0f);
        };
        auto color_from_gradient = [&](float u) -> ImU32 {
            const int num = 5;
            float pos = u * (num - 1);
            int i0 = (int)floorf(pos), i1 = i0 + 1;
            float t = pos - i0;
            if (i0 < 0)    { i0 = 0;       t = 0.0f; }
            if (i1 >= num) { i1 = num - 1; t = 0.0f; }
            return ImGui::ColorConvertFloat4ToU32(lerp_color(stops[i0], stops[i1], t));
        };

        float cur_x = title_x;
        for (const char* p = title; *p; p++) {
            char ch[2] = { *p, '\0' };
            ImVec2 csz = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, ch);
            float u = (cur_x + csz.x * 0.5f - title_x) / text_size.x;
            u = fmodf(u * 5.0f - phase * 5.0f, 1.0f);
            if (u < 0.0f) u += 1.0f;
            draw_list->AddText(ImVec2(cur_x, title_y), color_from_gradient(u), ch);
            cur_x += csz.x;
        }

        // Minimize button at top-right of title bar
        float fh = ImGui::GetFrameHeight();
        ImVec2 btnMin = ImVec2(wPos.x + wSz.x - 22.0f, wPos.y + 3.0f);
        ImVec2 btnMax = ImVec2(wPos.x + wSz.x - 4.0f,  wPos.y + fh - 3.0f);
        bool minHovered = ImGui::IsMouseHoveringRect(btnMin, btnMax, false);
        if (minHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            s_minimized = !s_minimized;
        draw_list->AddRectFilled(btnMin, btnMax, minHovered ? IM_COL32(200,200,200,160) : IM_COL32(120,120,120,100), 3.0f);
        float btnMidY = (btnMin.y + btnMax.y) * 0.5f;
        if (s_minimized)
            draw_list->AddText(ImVec2(btnMin.x + 4.0f, btnMin.y + 1.0f), IM_COL32(255,255,255,230), "+");
        else
            draw_list->AddLine(ImVec2(btnMin.x + 3.0f, btnMidY), ImVec2(btnMax.x - 3.0f, btnMidY), IM_COL32(255,255,255,230), 2.0f);
    }

    if (!s_minimized)
    {
    // Background image at 50% opacity, stretched to fill the window
    if (Hooks::g_pBackgroundSRV)
    {
        ImVec2 wPos = ImGui::GetWindowPos();
        ImVec2 wSz  = ImGui::GetWindowSize();
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)Hooks::g_pBackgroundSRV,
            wPos, ImVec2(wPos.x + wSz.x, wPos.y + wSz.y),
            ImVec2(0, 0), ImVec2(1, 1),
            IM_COL32(255, 255, 255, 26));
    }

    if (ImGui::BeginTabBar("MainTabBar")) {

        // ── Bots tab ───────────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Bots")) {
            ImGui::Spacing();

            if (ToggleRow("Pacify AI", "", bPacifyAI))
                { Patches::PacifyAI(bPacifyAI); SavePrefs(); }

            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        // ── Player tab ────────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Player")) {
            ImGui::Spacing();

            ImGui::Spacing();
            ImGui::TextDisabled("  Player Scale  (0 = default)");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::SliderFloat("##PScale", &fPlayerScale, -100.0f, 100.0f, "%.0f"))
                { Patches::g_PlayerScaleSlider = fPlayerScale; SavePrefs(); }

            ImGui::Spacing();
            ImGui::TextDisabled("  Camera FOV  (~90 = default)");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::SliderFloat("##FOV", &fCameraFOV, 0.0f, 160.0f, "%.0f"))
                { Patches::g_CameraFOV = fCameraFOV; SavePrefs(); }

            ImGui::Spacing();
            ImGui::TextDisabled("  Camera Zoom  (0 = off)");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::SliderFloat("##CamZoom", &fCameraZoom, 0.0f, 3.0f, "%.2f"))
                { Patches::g_CameraZoom = fCameraZoom; SavePrefs(); }

            ImGui::Spacing();
            if (ToggleRow("Auto claim Rewards", "", bAutoClaimRewards))
            {
                Patches::g_AutoClaimRewards = bAutoClaimRewards;
                if (bAutoClaimRewards) Patches::ClaimAllQuests();
                SavePrefs();
            }
            if (ToggleRow("Replace Kill Text", "", bPhoenixKillStreak))
            {
                Patches::KillStreakTextHook(bPhoenixKillStreak);
                SavePrefs();
            }
            if (ToggleRow("Hide Player Name (For recording)", "", bHidePlayerName)) { Patches::HidePlayerName(bHidePlayerName); SavePrefs(); }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextDisabled("  FPS Limit Override");
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputInt("##FPSOverride", &iFPSOverride, 10, 60))
            {
                if (iFPSOverride < -1) iFPSOverride = -1;
                Patches::g_FPSOverride = iFPSOverride;
                SavePrefs();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ToggleRow("Noclip", "", bNoclip))
                Patches::NoclipEnable(bNoclip);

            if (ToggleRow("Use Camera", "", bUseCameraDir))
                { Patches::g_UseCameraDir = bUseCameraDir; SavePrefs(); }

            if (ToggleRow("Force Grounded", "", bForceGrounded))
            {
                Patches::g_ForceGrounded = bForceGrounded;
                Patches::ForceGroundedHook(bForceGrounded);
                SavePrefs();
            }

            ImGui::Spacing();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Speed", &Patches::g_NoclipSpeed, 50.0f, 200.0f, "%.0f")) SavePrefs();
            if (Patches::g_NoclipSpeed < 1.0f) Patches::g_NoclipSpeed = 1.0f;

            ImGui::Spacing();
            {
                char bFwd[32], bBack[32], bRight[32], bLeft[32], bUp[32], bDown[32], bToggle[32];
                VKeyName(Patches::g_NcKeyFwd,    bFwd,    sizeof(bFwd));
                VKeyName(Patches::g_NcKeyBack,   bBack,   sizeof(bBack));
                VKeyName(Patches::g_NcKeyRight,  bRight,  sizeof(bRight));
                VKeyName(Patches::g_NcKeyLeft,   bLeft,   sizeof(bLeft));
                VKeyName(Patches::g_NcKeyUp,     bUp,     sizeof(bUp));
                VKeyName(Patches::g_NcKeyDown,   bDown,   sizeof(bDown));
                VKeyName(g_Binds[BIND_NOCLIP],   bToggle, sizeof(bToggle));
                ImGui::TextDisabled("  %s/%s/%s/%s + %s (up) / %s (down)  |  Toggle: %s",
                    bFwd, bBack, bRight, bLeft, bUp, bDown, bToggle);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ToggleRow("Auto Goto Enemy", "", bAutoGotoEnemy)) { Patches::g_AutoGotoEnemy = bAutoGotoEnemy; SavePrefs(); }
            if (ToggleRow("Ghost Goto Enemy", "", bGhostGotoEnemy)) { Patches::g_GhostGotoEnemy = bGhostGotoEnemy; Patches::GhostGotoEnemyHook(bGhostGotoEnemy); SavePrefs(); }
            if (ToggleRow("Invisible", "", bInvisible)) { Patches::g_Invisible = bInvisible; Patches::InvisibleHook(bInvisible); SavePrefs(); }
            if (ToggleRow("Ignore Absorber", "", bIgnoreAbsorber)) { Patches::g_IgnoreAbsorber = bIgnoreAbsorber; SavePrefs(); }
            if (ToggleRow("Ignore PhaseShift", "", bIgnorePhaseShift)) { Patches::g_IgnorePhaseShift = bIgnorePhaseShift; SavePrefs(); }
            if (ToggleRow("Ignore Reflectors", "", bIgnoreCounterShield)) { Patches::g_IgnoreCounterShield = bIgnoreCounterShield; SavePrefs(); }
            if (ToggleRow("Anti Stealth", "", bAntiStealth)) { Patches::g_AntiStealth = bAntiStealth; Patches::AntiEffectHook(bAntiStealth); SavePrefs(); }

            ImGui::Spacing();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Offset X", &Patches::g_GotoEnemyOffsetX, 1.0f, 10.0f, "%.1f")) SavePrefs();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Offset Y", &Patches::g_GotoEnemyOffsetY, 1.0f, 10.0f, "%.1f")) SavePrefs();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Offset Z", &Patches::g_GotoEnemyOffsetZ, 1.0f, 10.0f, "%.1f")) SavePrefs();

            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        // ── Buttons tab ───────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Buttons")) {
            ImGui::Spacing();
            ImGui::TextDisabled("Instant actions (These are a work in progress may not work as expected)");
            ImGui::Spacing();
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Jump Power", &fJumpPower, 1.0f, 10.0f, "%.0f"))
            {
                if (fJumpPower < 0.0f) fJumpPower = 0.0f;
                Patches::g_JumpPower = fJumpPower;
                SavePrefs();
            }
            ImGui::SetNextItemWidth(120.0f);
            if (ImGui::InputFloat("Boost Power", &fBoostPower, 1.0f, 10.0f, "%.0f"))
            {
                if (fBoostPower < 0.0f) fBoostPower = 0.0f;
                Patches::g_BoostPower = fBoostPower;
                SavePrefs();
            }
            ImGui::Spacing();
            if (ImGui::Button("Jump", ImVec2(-FLT_MIN, 0)))
                Patches::JumpUp();
            if (ImGui::Button("Boost Forward", ImVec2(-FLT_MIN, 0)))
                Patches::BoostForward();
            ImGui::Spacing();
            ImGui::TextDisabled("Bind these actions in the Keybinds tab.");
            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        // ── Weapons tab ───────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Weapons")) {
            ImGui::Spacing();
            if (ToggleRow("Auto Fire",         "", bAutoFire))       { Patches::g_AutoFire       = bAutoFire;       SavePrefs(); }
            if (ToggleRow("Rapid Fire (Mainly for PvE)",       "", bRapidFire))      { Patches::g_RapidFire      = bRapidFire;      SavePrefs(); }
            if (ToggleRow("Infinite Ammo (Mainly for PvE)",    "", bInfiniteAmmo))   { Patches::g_InfiniteAmmo   = bInfiniteAmmo;   SavePrefs(); }
            if (ToggleRow("Huge Explosions (Mainly for PvE)",  "", bHugeExplosions)) { Patches::g_HugeExplosions = bHugeExplosions; Patches::HugeExplosions(bHugeExplosions); SavePrefs(); }
            if (ToggleRow("Damage Boost (Mainly for PvE) (Detected in tutorial)", "", bDamageBoost))
            {
                Patches::g_DamageBoost = bDamageBoost;
                Patches::DamageBoostHook(bDamageBoost);
                SavePrefs();
            }
            if (bDamageBoost)
            {
                ImGui::SetNextItemWidth(160.0f);
                if (ImGui::DragFloat("Multiplier##dmgboost", &fDamageBoostMul, 0.5f, 1.0f, 1000.0f, "x%.1f"))
                {
                    Patches::g_DamageBoostMul = fDamageBoostMul;
                    SavePrefs();
                }
            }
            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        // ── Abilities tab ─────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Abilities")) {
            ImGui::Spacing();            if (ToggleRow("Remove Locked States", "", bNoLockedStates))
                { Patches::NoLockedStates(bNoLockedStates); SavePrefs(); }

            ImGui::Spacing();            ImGui::TextDisabled("Instant Cooldown");
            ImGui::Separator();
            if (ToggleRow("Ability 1 CD", "", bInst1CD))  { Patches::g_Instant1CD   = bInst1CD;  SavePrefs(); }
            if (ToggleRow("Ability 2 CD", "", bInst2CD))  { Patches::g_Instant2CD   = bInst2CD;  SavePrefs(); }
            if (ToggleRow("Module CD",    "", bInstModCD)) { Patches::g_InstantModCD = bInstModCD; SavePrefs(); }

            ImGui::Spacing();
            ImGui::TextDisabled("Duration Override  (seconds)");
            ImGui::Separator();
            if (DurRow("dur1", "Ability 1", bAbility1Dur, Patches::g_Ability1DurVal)) { Patches::g_Ability1Dur = bAbility1Dur; SavePrefs(); }
            if (DurRow("dur2", "Ability 2", bAbility2Dur, Patches::g_Ability2DurVal)) { Patches::g_Ability2Dur = bAbility2Dur; SavePrefs(); }
            if (DurRow("durm", "Module",    bModuleDur,   Patches::g_ModuleDurVal))   { Patches::g_ModuleDur   = bModuleDur;   SavePrefs(); }

            ImGui::Spacing();
            ImGui::TextDisabled("Auto-Use  (interval in seconds)");
            ImGui::Separator();
            if (AutoRow("au1", "Auto Ability 1", bAutoAbility1, Patches::g_AutoA1Interval))  { Patches::g_AutoAbility1 = bAutoAbility1; SavePrefs(); }
            if (AutoRow("au2", "Auto Ability 2", bAutoAbility2, Patches::g_AutoA2Interval))  { Patches::g_AutoAbility2 = bAutoAbility2; SavePrefs(); }
            if (AutoRow("aum", "Auto Module", bAutoModule, Patches::g_AutoModInterval))
            {
                Patches::g_AutoModule = bAutoModule;
                SavePrefs();
            }

            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        // ── Keybinds tab ──────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Keybinds")) {
            ImGui::Spacing();
            if (g_CapturingBind >= 0 || g_CapturingNcBind >= 0)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "  Press any key...  ESC = unbind");
            else
                ImGui::TextDisabled("  Click a button to assign a key.  ESC = unbind.");
            ImGui::Spacing();

            ImGui::TextDisabled("Bots");      ImGui::Separator();
            DrawBindRow("Pacify AI",             BIND_PACIFY_AI);
            ImGui::Spacing();

            ImGui::TextDisabled("Player");    ImGui::Separator();
            DrawBindRow("Hide Player Name (For recording)",      BIND_HIDE_NAME);
            DrawBindRow("Noclip",                BIND_NOCLIP);
            DrawBindRow("Auto Goto Enemy",       BIND_AUTO_ENEMY);
            DrawBindRow("Jump",                  BIND_JUMP);
            DrawBindRow("Boost Forward",         BIND_BOOST_FORWARD);
            ImGui::Spacing();

            ImGui::TextDisabled("Noclip Movement"); ImGui::Separator();
            DrawNcBindRow("Forward",    &Patches::g_NcKeyFwd,    0);
            DrawNcBindRow("Backward",   &Patches::g_NcKeyBack,   1);
            DrawNcBindRow("Right",      &Patches::g_NcKeyRight,  2);
            DrawNcBindRow("Left",       &Patches::g_NcKeyLeft,   3);
            DrawNcBindRow("Up",         &Patches::g_NcKeyUp,     4);
            DrawNcBindRow("Down",       &Patches::g_NcKeyDown,   5);
            DrawNcBindRow("Sprint",     &Patches::g_NcKeySprint, 6);
            ImGui::Spacing();

            ImGui::TextDisabled("Weapons");   ImGui::Separator();
            DrawBindRow("Rapid Fire (Mainly for PvE)",            BIND_RAPID_FIRE);
            DrawBindRow("Auto Fire",             BIND_AUTO_FIRE);
            DrawBindRow("Infinite Ammo (Mainly for PvE)",         BIND_INF_AMMO);
            DrawBindRow("Huge Explosions (Mainly for PvE)",       BIND_HUGE_EXP);
            ImGui::Spacing();

            ImGui::TextDisabled("Abilities"); ImGui::Separator();
            DrawBindRow("Remove Locked States",  BIND_NO_LOCKED);
            DrawBindRow("Ability 1 CD",          BIND_INST1CD);
            DrawBindRow("Ability 2 CD",          BIND_INST2CD);
            DrawBindRow("Module CD",             BIND_INSTMODCD);
            DrawBindRow("Ability 1 Duration",    BIND_ABILITY1DUR);
            DrawBindRow("Ability 2 Duration",    BIND_ABILITY2DUR);
            DrawBindRow("Module Duration",       BIND_MODULEDUR);
            DrawBindRow("Auto Ability 1",        BIND_AUTO_AB1);
            DrawBindRow("Auto Ability 2",        BIND_AUTO_AB2);
            DrawBindRow("Auto Module", BIND_AUTO_MOD);
            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        // ── Info tab ───────────────────────────────────────────────────────
        if (ImGui::BeginTabItem("Info")) {
            ImGui::Spacing();
            ImGui::Text("INSERT  --  toggle menu");

            // Animate: cycle hue between silver (low saturation, high value) and blue
            float t = (float)(GetTickCount64() % 3000) / 3000.0f; // 0..1 over 3 seconds
            // Silver: (0.6, 0.6, 0.8)  Blue: (0.2, 0.4, 1.0)
            // Ping-pong: remap t so it goes 0->1->0
            float ping = (t < 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
            float r = 0.6f + ping * (0.2f - 0.6f);   // 0.6 -> 0.2
            float g = 0.6f + ping * (0.4f - 0.6f);   // 0.6 -> 0.4
            float b = 0.8f + ping * (1.0f - 0.8f);   // 0.8 -> 1.0
            ImGui::TextColored(ImVec4(r, g, b, 1.0f), "");

            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    } // end !s_minimized

    ImGui::End();
}

