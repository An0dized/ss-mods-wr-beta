#pragma once
#include "pch.h"

namespace Patches {
    // Pacify AI
    bool PacifyAI(bool enable);

    // Remove Locked States (ability/module lock getters patched to always return false)
    bool NoLockedStates(bool enable);
    bool RotationSpeed5x(bool enable);
    bool InstantTargetLock(bool enable);
    bool AntiEffectHook(bool enable);

    // Huge Explosions (AoE radius stats getters patched + per-frame ECS setter calls)
    bool HugeExplosions(bool enable);

    // Damage Boost — writes weapon._internalDamageMultiplier (+0xA4) every frame
    // AND hooks CreateHitEventData to multiply DamageMultiplier in the outgoing hit packet.
    bool DamageBoostHook(bool enable);

    // Player name obfuscation (per-frame re-apply while active)
    bool HidePlayerName(bool enable);
    extern bool g_HidePlayerName;

    // Quest helper
    bool ClaimAllQuests();
    bool KillStreakTextHook(bool enable);
    bool JumpUp();
    bool BoostForward();
    extern float g_JumpPower;
    extern float g_BoostPower;
    extern bool g_AutoClaimRewards;

    // Noclip
    bool NoclipEnable(bool enable);
    void NoclipTick();
    extern bool  g_NoclipActive;
    extern bool  g_UseCameraDir;
    extern float g_NoclipSpeed;
    // Noclip movement keybinds (VK codes; default W/S/D/A/E/Q)
    extern int g_NcKeyFwd;
    extern int g_NcKeyBack;
    extern int g_NcKeyRight;
    extern int g_NcKeyLeft;
    extern int g_NcKeyUp;
    extern int g_NcKeyDown;
    extern int g_NcKeySprint;  // hold = 2x speed (default: VK_SHIFT)

    // Weapons
    extern int  g_FPSOverride;   // 0=off, -1=unlimited, >0=specific cap
    extern bool g_RapidFire;
    extern bool g_InfiniteAmmo;
    extern bool g_HugeExplosions;
    extern bool g_AutoFire;
    extern bool  g_DamageBoost;
    extern float g_DamageBoostMul;

    // Goto Enemy
    extern bool  g_AutoGotoEnemy;
    bool GhostGotoEnemyHook(bool enable);
    bool ForceGroundedHook(bool enable);
    bool InvisibleHook(bool enable);
    extern bool g_Invisible;
    extern bool g_ForceGrounded;
    extern bool  g_GhostGotoEnemy;
    extern bool  g_IgnoreAbsorber;
    extern bool  g_IgnorePhaseShift;
    extern bool  g_IgnoreCounterShield;
    extern bool  g_AntiStealth;
    extern bool  g_AntiEMP;
    extern bool  g_RotationSpeed5x;
    extern bool  g_InstantTargetLockSpeed;
    extern float g_GotoEnemyOffsetX;
    extern float g_GotoEnemyOffsetY;
    extern float g_GotoEnemyOffsetZ;

    // Ability / Module instant cooldown
    extern bool g_Instant1CD;
    extern bool g_Instant2CD;
    extern bool g_InstantModCD;

    // Ability / Module duration override
    extern bool  g_Ability1Dur;
    extern bool  g_Ability2Dur;
    extern bool  g_ModuleDur;
    extern float g_Ability1DurVal;
    extern float g_Ability2DurVal;
    extern float g_ModuleDurVal;

    // Auto-use (GotoNextState on timer)
    extern bool  g_AutoAbility1;
    extern bool  g_AutoAbility2;
    extern bool  g_AutoModule;
    extern float g_AutoA1Interval;
    extern float g_AutoA2Interval;
    extern float g_AutoModInterval;

    // Player size / FOV / Zoom override
    extern float g_PlayerScaleSlider;  // -100 to 100, 0 = default
    extern float g_CameraFOV;          // 0 = use game default (Cinemachine LensSettings.FieldOfView)
    extern float g_CameraZoom;         // 0 = disabled, 1 = default distance, >1 = farther, <1 = closer

    // Per-frame tick for all combat features (called from hkPresent).
    void GameplayTick();
}
