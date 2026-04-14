#include "pch.h"
#include "patches.h"
#include "MinHook.h"
#include <atomic>
#include <cmath>

// Append-only log for patch diagnostics (does NOT clear the file)
static void PLog(const char* fmt, ...)
{
    char msg[512];
    va_list va; va_start(va, fmt); _vsnprintf_s(msg, sizeof(msg), _TRUNCATE, fmt, va); va_end(va);
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    char* slash = strrchr(path, '\\'); if (slash) *(slash + 1) = '\0';
    strncat_s(path, "WarRobotsMod.log", 16);
    FILE* f = nullptr; fopen_s(&f, path, "a");
    if (f) { fprintf(f, "[Patches] %s\n", msg); fclose(f); }
}

// -------------------------------------------------------
// GameAssembly.dll base (resolved once)
// -------------------------------------------------------
static uintptr_t GetBase() {
    static uintptr_t base = 0;
    if (!base)
        base = (uintptr_t)GetModuleHandleA("GameAssembly.dll");
    return base;
}

static uintptr_t A(uintptr_t rva) { return GetBase() + rva; }

// Allocate an IL2CPP managed string and immediately register a permanent strong GC handle
// so the object is not collected even though only a native static holds the pointer.
static void* MakePinnedString(const char* utf8)
{
    typedef void*    (__cdecl* fn_StrNew_t)  (const char*);
    typedef uint32_t (__cdecl* fn_GcHandle_t)(void*, bool);
    auto fnStrNew   = (fn_StrNew_t)  GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");
    auto fnGcHandle = (fn_GcHandle_t)GetProcAddress((HMODULE)GetBase(), "il2cpp_gchandle_new");
    if (!fnStrNew) return nullptr;
    void* str = nullptr;
    __try { str = fnStrNew(utf8); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (str && fnGcHandle)
        __try { fnGcHandle(str, false); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return str;
}

typedef void* (__cdecl* fn_il2cpp_object_new)(void* klass);
typedef void* (__cdecl* fn_il2cpp_class_get_method_from_name)(void* klass, const char* name, int args);
typedef void* (__cdecl* fn_il2cpp_runtime_invoke)(void* method, void* obj, void** params, void** exc);
typedef void* (__cdecl* fn_domain_get)();
typedef void** (__cdecl* fn_domain_get_assemblies)(void* domain, size_t* count);
typedef void* (__cdecl* fn_assembly_get_image)(void* assembly);
typedef void* (__cdecl* fn_class_from_name)(void* image, const char* ns, const char* name);

static bool s_questAPIInited = false;
static bool s_questAPIFailed = false;
static void* s_ListStringClass = nullptr;
static void* s_ListStringCtor = nullptr;
static void* s_ListStringAdd = nullptr;
static fn_il2cpp_object_new s_fnObjectNew = nullptr;
static fn_il2cpp_class_get_method_from_name s_fnClassGetMethodFromName = nullptr;
static fn_il2cpp_runtime_invoke s_fnRuntimeInvoke = nullptr;
static void* (__fastcall* s_fnGetQuestService)(void* method) = nullptr;
static void* (__fastcall* s_fnTryFinishAllQuests)(void* self, void* questTabs, void* method) = nullptr;
static void* (*s_fnGetPveModel)() = nullptr;
static void* (__fastcall* s_fnPveModelGetState)(void* self, void* method) = nullptr;
static long long (__fastcall* s_fnPveUtilsGetLayout)(void* playerState) = nullptr;
static void* (__fastcall* s_fnPve20RewardAllAsync)(void* request, void* cancellationToken, void* member, void* file, int line) = nullptr;

static void* ResolveIl2CppClass(const char* ns, const char* name)
{
    HMODULE hGA = (HMODULE)GetBase();
    if (!hGA) return nullptr;

    auto fnDomainGet     = (fn_domain_get)            GetProcAddress(hGA, "il2cpp_domain_get");
    auto fnDomainGetAsms = (fn_domain_get_assemblies) GetProcAddress(hGA, "il2cpp_domain_get_assemblies");
    auto fnAsmGetImage   = (fn_assembly_get_image)    GetProcAddress(hGA, "il2cpp_assembly_get_image");
    auto fnClassFromName = (fn_class_from_name)       GetProcAddress(hGA, "il2cpp_class_from_name");
    if (!fnDomainGet || !fnDomainGetAsms || !fnAsmGetImage || !fnClassFromName)
        return nullptr;

    void* domain = fnDomainGet();
    if (!domain) return nullptr;

    size_t asmCount = 0;
    void** assemblies = fnDomainGetAsms(domain, &asmCount);
    if (!assemblies || !asmCount) return nullptr;

    for (size_t i = 0; i < asmCount; i++)
    {
        void* img = fnAsmGetImage(assemblies[i]);
        if (!img) continue;
        void* klass = fnClassFromName(img, ns, name);
        if (klass) return klass;
    }
    return nullptr;
}

static bool InitQuestAPI()
{
    if (!GetBase()) return false;
    HMODULE hGA = (HMODULE)GetBase();
    if (!hGA) return false;

    if (!s_fnObjectNew) s_fnObjectNew = (fn_il2cpp_object_new)GetProcAddress(hGA, "il2cpp_object_new");
    if (!s_fnClassGetMethodFromName) s_fnClassGetMethodFromName = (fn_il2cpp_class_get_method_from_name)GetProcAddress(hGA, "il2cpp_class_get_method_from_name");
    if (!s_fnRuntimeInvoke) s_fnRuntimeInvoke = (fn_il2cpp_runtime_invoke)GetProcAddress(hGA, "il2cpp_runtime_invoke");
    if (!s_fnObjectNew || !s_fnClassGetMethodFromName || !s_fnRuntimeInvoke)
        return false;

    s_fnGetQuestService = (void* (__fastcall*)(void* method))(GetBase() + 0x83490C0);
    s_fnTryFinishAllQuests = (void* (__fastcall*)(void* self, void* questTabs, void* method))(GetBase() + 0x11915A0);
    s_fnGetPveModel = (void* (*)())(GetBase() + 0x8347DA0);
    s_fnPveModelGetState = (void* (__fastcall*)(void* self, void* method))(GetBase() + 0x7E6340);
    s_fnPveUtilsGetLayout = (int64_t (__fastcall*)(void* playerState))(GetBase() + 0x1230960);
    s_fnPve20RewardAllAsync = (void* (__fastcall*)(void* request, void* cancellationToken, void* member, void* file, int line))(GetBase() + 0x50BDAE0);
    if (!s_fnGetQuestService || !s_fnTryFinishAllQuests)
        return false;

    if (!s_ListStringClass)
    {
        void* listKlass = ResolveIl2CppClass("System.Collections.Generic", "List`1");
        if (!listKlass)
            listKlass = ResolveIl2CppClass("System.Collections.Generic", "List");
        if (!listKlass) return false;

        void* ctor = s_fnClassGetMethodFromName(listKlass, ".ctor", 0);
        void* add  = s_fnClassGetMethodFromName(listKlass, "Add", 1);
        if (!ctor || !add) return false;

        s_ListStringClass = listKlass;
        s_ListStringCtor  = ctor;
        s_ListStringAdd   = add;
    }

    return true;
}

static bool EnsureQuestAPI()
{
    if (!s_questAPIInited)
    {
        s_questAPIInited = true;
        s_questAPIFailed = !InitQuestAPI();
    }
    return !s_questAPIFailed;
}

static void* CreateStringList(const char* const* values, int count)
{
    if (!s_fnObjectNew || !s_ListStringClass || !s_ListStringCtor || !s_ListStringAdd)
        return nullptr;

    void* list = s_fnObjectNew(s_ListStringClass);
    if (!list) return nullptr;

    __try { s_fnRuntimeInvoke(s_ListStringCtor, list, nullptr, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }

    for (int i = 0; i < count; i++)
    {
        void* str = MakePinnedString(values[i]);
        if (!str) continue;
        void* args[1] = { str };
        __try { s_fnRuntimeInvoke(s_ListStringAdd, list, args, nullptr); }
        __except (EXCEPTION_EXECUTE_HANDLER) { }
    }

    return list;
}

static const char* s_QuestTabs[] = {
    "Special", "Event", "Bonus", "Daily", "Clan", "Title",
    "Battlepass_A", "Battlepass_C", "Battlepass_D", "Battlepass_E", "Battlepass_R", "Returnee"
};

static bool ClaimAllPvERewards()
{
    if (!EnsureQuestAPI()) return false;
    if (!s_fnGetPveModel || !s_fnPveModelGetState || !s_fnPveUtilsGetLayout || !s_fnPve20RewardAllAsync)
        return false;

    void* pveModel = nullptr;
    __try { pveModel = s_fnGetPveModel(); } __except (EXCEPTION_EXECUTE_HANDLER) { pveModel = nullptr; }
    if (!pveModel) return false;

    void* pveState = nullptr;
    __try { pveState = s_fnPveModelGetState(pveModel, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { pveState = nullptr; }
    if (!pveState) return false;

    int64_t layout = 0;
    __try { layout = s_fnPveUtilsGetLayout(pveState); } __except (EXCEPTION_EXECUTE_HANDLER) { layout = 0; }
    if (layout <= 0) return false;

    void* requestKlass = ResolveIl2CppClass("BattleMechs.Client.Message.GameMode.PvE20", "PvECollectAllRewardsRequestMessage");
    if (!requestKlass) return false;

    void* ctor = s_fnClassGetMethodFromName(requestKlass, ".ctor", 0);
    if (!ctor) return false;

    void* request = s_fnObjectNew(requestKlass);
    if (!request) return false;

    __try { s_fnRuntimeInvoke(ctor, request, nullptr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    __try { *(int64_t*)((char*)request + 0x10) = layout; } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    __try { s_fnPve20RewardAllAsync(request, nullptr, nullptr, nullptr, 0); } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    return true;
}

static void* ResolveIl2CppClassFromCandidates(const char* const* namespaces, const char* name)
{
    for (const char* const* ns = namespaces; *ns; ++ns)
    {
        void* klass = ResolveIl2CppClass(*ns, name);
        if (klass) return klass;
    }
    return nullptr;
}

static bool ClaimAllBattleSeasonRewards()
{
    if (!EnsureQuestAPI() || !s_fnClassGetMethodFromName || !s_fnRuntimeInvoke)
        return false;

    static const char* const contextNamespaces[] = {
        "Game.Core",
        "BattleMechs.Client",
        "BattleMechs.Client.Message",
        nullptr
    };
    void* contextKlass = ResolveIl2CppClassFromCandidates(contextNamespaces, "MenuContext");
    if (!contextKlass) return false;

    void* getModel = s_fnClassGetMethodFromName(contextKlass, "get_BattleSeasonModel", 0);
    if (!getModel) return false;

    void* model = nullptr;
    __try { model = s_fnRuntimeInvoke(getModel, nullptr, nullptr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { model = nullptr; }
    if (!model) return false;

    static const char* const modelNamespaces[] = {
        "BattleMechs.Client",
        "BattleMechs.Client.Model",
        "BattleMechs.Client.Message",
        "BattleMechs.Client.Message.Battleseason",
        nullptr
    };
    void* modelKlass = ResolveIl2CppClassFromCandidates(modelNamespaces, "BattleSeasonModel");
    if (!modelKlass) return false;

    void* getActiveSeasons = s_fnClassGetMethodFromName(modelKlass, "get_ActiveSeasons", 0);
    if (!getActiveSeasons) return false;

    void* activeSeasons = nullptr;
    __try { activeSeasons = s_fnRuntimeInvoke(getActiveSeasons, model, nullptr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { activeSeasons = nullptr; }
    if (!activeSeasons) return false;

    void* listKlass = ResolveIl2CppClass("System.Collections.Generic", "List`1");
    if (!listKlass) return false;

    void* getItem = s_fnClassGetMethodFromName(listKlass, "get_Item", 1);
    if (!getItem) return false;

    int index = 0;
    void* itemArgs[1] = { &index };
    void* seasonMessage = nullptr;
    __try { seasonMessage = s_fnRuntimeInvoke(getItem, activeSeasons, itemArgs, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { seasonMessage = nullptr; }
    if (!seasonMessage) return false;

    void* seasonIdString = nullptr;
    static const char* const seasonMessageNamespaces[] = {
        "BattleMechs.Client.Message",
        "BattleMechs.Client.Message.Battleseason",
        "BattleMechs.Client",
        nullptr
    };
    void* seasonMessageKlass = ResolveIl2CppClassFromCandidates(seasonMessageNamespaces, "PlayerBattleSeasonMessage");
    if (seasonMessageKlass)
    {
        void* getSeasonId = s_fnClassGetMethodFromName(seasonMessageKlass, "get_SeasonId", 0);
        if (getSeasonId)
        {
            __try { seasonIdString = s_fnRuntimeInvoke(getSeasonId, seasonMessage, nullptr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { seasonIdString = nullptr; }
        }
    }
    if (!seasonIdString)
    {
        __try { seasonIdString = *(void**)((char*)seasonMessage + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) { seasonIdString = nullptr; }
    }
    if (!seasonIdString) return false;

    void* getService = s_fnClassGetMethodFromName(contextKlass, "get_BattleSeason", 0);
    if (!getService) return false;

    void* service = nullptr;
    __try { service = s_fnRuntimeInvoke(getService, nullptr, nullptr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { service = nullptr; }
    if (!service) return false;

    static const char* const serviceNamespaces[] = {
        "BattleMechs.Client",
        "BattleMechs.Client.Service",
        "BattleMechs.Client.Message",
        "BattleMechs.Client.Message.Battleseason",
        nullptr
    };
    void* serviceKlass = ResolveIl2CppClassFromCandidates(serviceNamespaces, "BattleSeasonService");
    if (!serviceKlass) return false;

    void* collectAll = s_fnClassGetMethodFromName(serviceKlass, "CollectAll", 1);
    if (!collectAll) return false;

    void* collectArgs[1] = { seasonIdString };
    __try { s_fnRuntimeInvoke(collectAll, service, collectArgs, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    return true;
}

bool Patches::ClaimAllQuests()
{
    if (!EnsureQuestAPI()) return false;
    if (!s_fnGetQuestService || !s_fnTryFinishAllQuests) return false;

    void* questService = nullptr;
    __try { questService = s_fnGetQuestService(nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { questService = nullptr; }
    if (!questService) return false;

    void* questTabs = CreateStringList(s_QuestTabs, _countof(s_QuestTabs));
    if (!questTabs)
    {
        // Fallback: try calling with null if list creation failed.
        __try { s_fnTryFinishAllQuests(questService, nullptr, nullptr); }
        __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
        ClaimAllPvERewards();
        ClaimAllBattleSeasonRewards();
        return true;
    }

    bool questResult = false;
    __try { s_fnTryFinishAllQuests(questService, questTabs, nullptr); questResult = true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { questResult = false; }
    bool pveResult = ClaimAllPvERewards();
    bool battleSeasonResult = ClaimAllBattleSeasonRewards();
    return questResult || pveResult || battleSeasonResult;
}

static const uintptr_t k_PhoenixKillStreakRVA = 0xDBF860;
static const uintptr_t k_PhoenixKillStreakGetLocKeyRVA = 0x1099D40;
static bool s_phoenixKillStreakHookCreated = false;
static bool s_phoenixKillStreakLocKeyHookCreated = false;
static void* s_phoenixKillStreakTarget = nullptr;
static void* s_phoenixKillStreakGetLocKeyTarget = nullptr;
static void* s_phoenixKillStreakText = nullptr;

typedef void (__fastcall* fn_MultiKillView_SetText_t)(void* self, void* text, void* method);
static fn_MultiKillView_SetText_t s_origMultiKillViewSetText = nullptr;

typedef void* (__fastcall* fn_MultiKillService_GetLocKey_t)(void* killData, void* method);
static fn_MultiKillService_GetLocKey_t s_origMultiKillServiceGetLocKey = nullptr;

static void __fastcall hkMultiKillViewSetText(void* self, void* text, void* method)
{
    if (!s_phoenixKillStreakText)
        s_phoenixKillStreakText = MakePinnedString("<b><color=#ff0000>T</color><color=#FF2500>i</color><color=#FF4A00>t</color><color=#FF6F00>a</color><color=#FF9400>n</color><color=#FFBA00>C</color><color=#FFDF00>o</color><color=#F4FF00>r</color><color=#AAFF00>e</color><color=#60FF00> </color><color=#15FF00>B</color><color=#00FF35>e</color><color=#00FF80>s</color><color=#00FFCA>t</color><color=#00EAFF> </color><color=#009FFF>C</color><color=#0055FF>h</color><color=#000BFF>e</color><color=#2300FF>a</color><color=#4B00FF>t</color><color=#7400FF> </color><color=#9A00DF>E</color><color=#BB0095>v</color><color=#DD004A>e</color><color=#ff0000>r</color></b>");

    if (s_phoenixKillStreakText)
    {
        s_origMultiKillViewSetText(self, s_phoenixKillStreakText, method);
        return;
    }
    s_origMultiKillViewSetText(self, text, method);
}

static void* __fastcall hkMultiKillServiceGetLocKey(void* killData, void* method)
{
    if (!s_phoenixKillStreakText)
        s_phoenixKillStreakText = MakePinnedString("<b><color=#ff0000>T</color><color=#FF2500>i</color><color=#FF4A00>t</color><color=#FF6F00>a</color><color=#FF9400>n</color><color=#FFBA00>C</color><color=#FFDF00>o</color><color=#F4FF00>r</color><color=#AAFF00>e</color><color=#60FF00> </color><color=#15FF00>B</color><color=#00FF35>e</color><color=#00FF80>s</color><color=#00FFCA>t</color><color=#00EAFF> </color><color=#009FFF>C</color><color=#0055FF>h</color><color=#000BFF>e</color><color=#2300FF>a</color><color=#4B00FF>t</color><color=#7400FF> </color><color=#9A00DF>E</color><color=#BB0095>v</color><color=#DD004A>e</color><color=#ff0000>r</color></b>");

    if (s_phoenixKillStreakText)
        return s_phoenixKillStreakText;
    return s_origMultiKillServiceGetLocKey(killData, method);
}

bool Patches::KillStreakTextHook(bool enable)
{
    if (!GetBase()) return false;

    if (!s_phoenixKillStreakHookCreated)
    {
        s_phoenixKillStreakTarget = reinterpret_cast<void*>(A(k_PhoenixKillStreakRVA));
        void* orig = nullptr;
        if (MH_CreateHook(s_phoenixKillStreakTarget, reinterpret_cast<void*>(&hkMultiKillViewSetText), &orig) != MH_OK)
            return false;
        s_origMultiKillViewSetText = reinterpret_cast<fn_MultiKillView_SetText_t>(orig);
        s_phoenixKillStreakHookCreated = true;
    }

    if (!s_phoenixKillStreakLocKeyHookCreated)
    {
        s_phoenixKillStreakGetLocKeyTarget = reinterpret_cast<void*>(A(k_PhoenixKillStreakGetLocKeyRVA));
        void* orig = nullptr;
        if (MH_CreateHook(s_phoenixKillStreakGetLocKeyTarget, reinterpret_cast<void*>(&hkMultiKillServiceGetLocKey), &orig) != MH_OK)
            return false;
        s_origMultiKillServiceGetLocKey = reinterpret_cast<fn_MultiKillService_GetLocKey_t>(orig);
        s_phoenixKillStreakLocKeyHookCreated = true;
    }

    if (enable)
    {
        MH_QueueEnableHook(s_phoenixKillStreakTarget);
        MH_QueueEnableHook(s_phoenixKillStreakGetLocKeyTarget);
    }
    else
    {
        MH_QueueDisableHook(s_phoenixKillStreakTarget);
        MH_QueueDisableHook(s_phoenixKillStreakGetLocKeyTarget);
    }
    MH_ApplyQueued();
    return true;
}


// -------------------------------------------------------
// Pacify AI patch sites
//
// Covers every code path bots use to fire weapons:
//   1. BotShooter.Shoot(int count, Tags)                  RVA 0x13DEB50  public � legacy + UtilityAI
//   2. BotShooter.Shoot(int slot, int count, Tags)        RVA 0x13DEAB0  public � legacy + UtilityAI
//   3. BotShooter.Shoot(IEquipmentSlot, int, Tags)        RVA 0x13DE970  static helper called by both
//   4. ShootExecutor.Tick(Context, ActionContext)          RVA 0x13E80A0  UtilityAI executor tick
//   5. BotMechController.ShootWeaponInRange(Target)       RVA 0xB7D480   legacy direct-shoot call
//   6. TowerExecutor.Tick(Context, ActionContext)          RVA 0x13E9060  tower-bot shoot executor
// -------------------------------------------------------
static const uintptr_t k_PacifyRVAs[] = {
    0x13DEB50,   // BotShooter.Shoot(count, tags)
    0x13DEAB0,   // BotShooter.Shoot(slot, count, tags)
    0x13DE970,   // BotShooter.Shoot � static helper
    0x13E80A0,   // ShootExecutor.Tick
    0xB7D480,    // BotMechController.ShootWeaponInRange
    0x13E9060,   // TowerExecutor.Tick
};

// No-op hook: all pacify targets are void IL2CPP methods; just return immediately.
// MinHook freezes threads before patching, so this is safe to toggle at any time.
static void __fastcall hkBotNoOp() { }
static bool s_pacifyHooksCreated = false;

bool Patches::PacifyAI(bool enable)
{
    if (!GetBase()) return false;

    // Create hooks once (lazy init)
    if (!s_pacifyHooksCreated)
    {
        int created = 0;
        for (uintptr_t rva : k_PacifyRVAs)
        {
            void* target = reinterpret_cast<void*>(A(rva));
            void* orig   = nullptr;
            if (MH_CreateHook(target, reinterpret_cast<void*>(&hkBotNoOp), &orig) != MH_OK)
            {
                for (int r = 0; r < created; r++)
                    MH_RemoveHook(reinterpret_cast<void*>(A(k_PacifyRVAs[r])));
                return false;
            }
            ++created;
        }
        s_pacifyHooksCreated = true;
    }

    // Queue all hooks then apply atomically (MinHook freezes threads for us)
    for (uintptr_t rva : k_PacifyRVAs)
    {
        void* target = reinterpret_cast<void*>(A(rva));
        if (enable) MH_QueueEnableHook(target);
        else        MH_QueueDisableHook(target);
    }
    MH_ApplyQueued();

    return true;
}


// -----------------------------------------------------------------------
// Remove Locked States
//
// Patches the four MechStats bool getters to always return false so
// abilities and modules can never enter a "locked" state.
//
//   MechStats.get_Ability1LockedByPilot         RVA 0xF26D80
//   MechStats.get_Ability2LockedByPilot         RVA 0xF26DC0
//   MechStats.get_AbilityLockedByStatusEffect   RVA 0xF26DE0
//   MechStats.get_ModuleLockedByStatusEffect    RVA 0xF26E60
//
// Cave: xor eax,eax ; ret  � returns false, stack untouched (jumps from
// function entry before any prologue runs, so ret pops the correct address).
// -----------------------------------------------------------------------
static const uintptr_t k_NoLockRVAs[] = {
    0xF26D80,   // MechStats.get_Ability1LockedByPilot
    0xF26DC0,   // MechStats.get_Ability2LockedByPilot
    0xF26DE0,   // MechStats.get_AbilityLockedByStatusEffect
    0xF26E60,   // MechStats.get_ModuleLockedByStatusEffect
};

// bool getter → always returns false so abilities/modules never enter locked state.
static bool __fastcall hkRetFalse() { return false; }
static bool s_noLockHooksCreated = false;
static bool s_rotationSpeedHooksCreated = false;
static bool s_targetLockHookCreated = false;
typedef bool  (__fastcall* fn_HasEffect)       (void* actor, int effectKind, void* method);
typedef float (__fastcall* fn_GetFloat_t)       (void* self, void* method);
static fn_HasEffect         o_HasEffect      = nullptr;
static bool                 s_hasEffectHookCreated = false;
static fn_GetFloat_t        o_GetAnimRotationSpeed = nullptr;
static fn_GetFloat_t        o_GetTurretHorizRotationSpeed = nullptr;
static fn_GetFloat_t        o_GetTargetLockSpeed = nullptr;
static void* GetActiveMech();

static float __fastcall hk_GetAnimRotationSpeed(void* self, void* method)
{
    float orig = 0.0f;
    if (o_GetAnimRotationSpeed)
        orig = o_GetAnimRotationSpeed(self, method);
    return Patches::g_RotationSpeed5x ? orig * 5.0f : orig;
}

static float __fastcall hk_GetTurretHorizRotationSpeed(void* self, void* method)
{
    float orig = 0.0f;
    if (o_GetTurretHorizRotationSpeed)
        orig = o_GetTurretHorizRotationSpeed(self, method);
    return Patches::g_RotationSpeed5x ? orig * 5.0f : orig;
}

static float __fastcall hk_GetTargetLockSpeed(void* self, void* method)
{
    float orig = 0.0f;
    if (o_GetTargetLockSpeed)
        orig = o_GetTargetLockSpeed(self, method);
    return Patches::g_InstantTargetLockSpeed ? 9999.0f : orig;
}

bool Patches::RotationSpeed5x(bool enable)
{
    if (!GetBase()) return false;
    if (!s_rotationSpeedHooksCreated)
    {
        void* target1 = reinterpret_cast<void*>(A(0x10D89F0)); // AnimationController.get_RotationSpeed
        void* target2 = reinterpret_cast<void*>(A(0x1029BE0)); // Turret.get_horizRotationSpeed
        if (MH_CreateHook(target1, reinterpret_cast<void*>(&hk_GetAnimRotationSpeed), reinterpret_cast<void**>(&o_GetAnimRotationSpeed)) != MH_OK)
            return false;
        if (MH_CreateHook(target2, reinterpret_cast<void*>(&hk_GetTurretHorizRotationSpeed), reinterpret_cast<void**>(&o_GetTurretHorizRotationSpeed)) != MH_OK)
            return false;
        s_rotationSpeedHooksCreated = true;
    }

    if (enable) {
        MH_QueueEnableHook(reinterpret_cast<void*>(A(0x10D89F0)));
        MH_QueueEnableHook(reinterpret_cast<void*>(A(0x1029BE0)));
    } else {
        MH_QueueDisableHook(reinterpret_cast<void*>(A(0x10D89F0)));
        MH_QueueDisableHook(reinterpret_cast<void*>(A(0x1029BE0)));
    }
    MH_ApplyQueued();
    return true;
}

bool Patches::InstantTargetLock(bool enable)
{
    if (!GetBase()) return false;
    if (!s_targetLockHookCreated)
    {
        void* target = reinterpret_cast<void*>(A(0xF1AAD0)); // MechStats.get_TargetLockSpeed
        if (MH_CreateHook(target, reinterpret_cast<void*>(&hk_GetTargetLockSpeed), reinterpret_cast<void**>(&o_GetTargetLockSpeed)) != MH_OK)
            return false;
        s_targetLockHookCreated = true;
    }

    void* target = reinterpret_cast<void*>(A(0xF1AAD0));
    if (enable) MH_QueueEnableHook(target);
    else        MH_QueueDisableHook(target);
    MH_ApplyQueued();
    return true;
}

static bool __fastcall hk_HasEffect(void* actor, int effectKind, void* method)
{
    if (Patches::g_AntiStealth && effectKind == 60)
        return false;
    if (o_HasEffect)
        return o_HasEffect(actor, effectKind, method);
    return false;
}

bool Patches::AntiEffectHook(bool enable)
{
    if (!GetBase()) return false;
    void* target = reinterpret_cast<void*>(A(0xB0B320)); // ActorExtensions.HasEffect(IActor, EffectKind)
    if (!s_hasEffectHookCreated)
    {
        if (MH_CreateHook(target, reinterpret_cast<void*>(&hk_HasEffect), reinterpret_cast<void**>(&o_HasEffect)) != MH_OK)
            return false;
        s_hasEffectHookCreated = true;
    }

    if (enable) MH_QueueEnableHook(target);
    else        MH_QueueDisableHook(target);
    MH_ApplyQueued();
    return true;
}

bool Patches::NoLockedStates(bool enable)
{
    if (!GetBase()) return false;

    if (!s_noLockHooksCreated)
    {
        int created = 0;
        for (uintptr_t rva : k_NoLockRVAs)
        {
            void* target = reinterpret_cast<void*>(A(rva));
            void* orig   = nullptr;
            if (MH_CreateHook(target, reinterpret_cast<void*>(&hkRetFalse), &orig) != MH_OK)
            {
                for (int r = 0; r < created; r++)
                    MH_RemoveHook(reinterpret_cast<void*>(A(k_NoLockRVAs[r])));
                return false;
            }
            ++created;
        }
        s_noLockHooksCreated = true;
    }

    for (uintptr_t rva : k_NoLockRVAs)
    {
        void* target = reinterpret_cast<void*>(A(rva));
        if (enable) MH_QueueEnableHook(target);
        else        MH_QueueDisableHook(target);
    }
    MH_ApplyQueued();
    return true;
}


// -----------------------------------------------------------------------
// Noclip  �  Position freeze + WASD / Q / E flight
//
// Pointer chain (all offsets confirmed from dump):
//   MatchContext.MechManagement  (static, namespace "Game.Core")
//     ?  MechManagementService._playerMechState   +0x30
//        ?  PlayerMechState.IsMechActive           +0x40  (bool)
//           PlayerMechState.ActiveMech             +0x30  (IActor*/Mech*)
//             ?  Mech.<rigidbody>k__BackingField   +0xC8  (Rigidbody wrapper*)
//
// Game RVAs (GameAssembly.dll):
//   Actor.get_Transform()         0xA23D30
//   Component.get_transform()     0x4DBF6E0  � returns Transform* (for Camera)
//   Transform.get_position()      0x4D3FB70  � Vector3 hidden-return (MSVC x64)
//   Transform.set_position()      0x4D403C0  � Vector3 passed by pointer
//   Transform.get_forward()       0x4D3F610  � Vector3 hidden-return
//   Transform.get_right()         0x4D3FBC0  � Vector3 hidden-return
//   Rigidbody.set_velocity()      0x4DBD310  � Vector3 passed by pointer
//   Camera.get_main()             0x4D87890  � static, returns Camera* in RAX
// -----------------------------------------------------------------------

struct Vector3 { float x, y, z; };

// IL2CPP exported API resolved at runtime via GetProcAddress
typedef void*  (__cdecl* fn_domain_get)();
typedef void** (__cdecl* fn_domain_get_assemblies)(void* domain, size_t* count);
typedef void*  (__cdecl* fn_assembly_get_image)(void* assembly);
typedef void*  (__cdecl* fn_class_from_name)(void* image, const char* ns, const char* name);
typedef void*  (__cdecl* fn_field_from_name)(void* klass, const char* name);
typedef void   (__cdecl* fn_field_static_get)(void* field, void* value);

// IL2CPP compiled game methods � Windows x64 MSVC calling convention
//   Actor.get_Transform / Component.get_transform (this*, MethodInfo*) ? Transform*/Camera*
typedef void* (__fastcall* fn_GetTransform)(void* self,       void* method);
//   Camera.get_main(MethodInfo*)  � static, no 'this'
typedef void* (__fastcall* fn_GetMainCamera)(void* method);
//   Transform.get_position / get_forward / get_right (Vector3* hiddenRet, Transform* this, MethodInfo*)
typedef void  (__fastcall* fn_GetVec3)     (Vector3* ret,     void* transform, void* method);
//   Transform.set_position(Transform* this, Vector3* byPtr,   MethodInfo*)
typedef void  (__fastcall* fn_SetPosition) (void* transform,  Vector3* value, void* method);
//   Rigidbody.set_velocity(Rigidbody* this, Vector3* byPtr,   MethodInfo*)
typedef void  (__fastcall* fn_SetVelocity)    (void* rigidbody,  Vector3* velocity, void* method);
//   Rigidbody.get_velocity_Injected(Rigidbody* this, Vector3* outRet, MethodInfo*)
typedef void  (__fastcall* fn_GetVelocityInjected)    (void* rigidbody, Vector3* ret, void* method);
//   ActorExtensions.HasEffect(IActor actor, EffectKind kind)
typedef bool  (__fastcall* fn_HasEffect)       (void* actor, int effectKind, void* method);
typedef float (__fastcall* fn_GetFloat_t)       (void* self, void* method);
//   Transform.set_localScale_Injected(Transform*, Vector3*, MethodInfo*)
typedef void  (__fastcall* fn_SetLocalScale_t) (void* transform,  Vector3* value,    void* method);
//   Camera.set_fieldOfView(Camera*, float, MethodInfo*)
typedef void  (__fastcall* fn_SetFOV_t)        (void* camera,     float value,       void* method);
//   Player.get_name() -> Il2CppString*  (static, no 'this')
typedef void* (__fastcall* fn_GetPlayerName_t) (void* method);
//   Player.set_name(Il2CppString*)  (static, no 'this')
typedef void  (__fastcall* fn_SetPlayerName_t) (void* str,        void* method);
//   Actor.set_ActorName(Il2CppString*)  (instance method: this, value, method)
typedef void  (__fastcall* fn_SetActorName_t)  (void* instance,   void* str,   void* method);
//   il2cpp_string_new -- C export, creates Il2CppString from UTF-8 C string
typedef void* (__cdecl*    fn_Il2cppStrNew_t)  (const char* str);

// -- Resolved pointers ------------------------------------------------
static fn_field_static_get g_FieldStaticGet  = nullptr;
static void*               g_MechMgmtField   = nullptr;   // Il2CppField*
static void*               g_NetworkSvcField = nullptr;   // Il2CppField* for MatchContext.NetworkService
static void*               g_LoginSvcField   = nullptr;   // Il2CppField* for PlayerContext.PlayerLoginService
static void*               g_SquadMgrField   = nullptr;   // Il2CppField* for PlayerContext._squadManager
static void*               g_DuelMgrField    = nullptr;   // Il2CppField* for PlayerContext.DuelManager
static fn_GetTransform     g_GetTransform    = nullptr;   // Actor.get_Transform
static fn_GetTransform     g_GetCompTransform = nullptr;  // Component.get_transform
static fn_GetMainCamera    g_GetMainCamera   = nullptr;
static fn_GetVec3          g_GetPosition     = nullptr;
static fn_GetVec3          g_GetForward      = nullptr;
static fn_GetVec3          g_GetRight        = nullptr;
static fn_SetPosition      g_SetPosition     = nullptr;
static fn_SetVelocity          g_SetVelocity         = nullptr;
static fn_GetVelocityInjected g_GetVelocityInjected = nullptr;
static fn_HasEffect         g_HasEffect      = nullptr;
static fn_GetVec3              g_GetLocalScale       = nullptr;  // Transform.get_localScale_Injected
static fn_SetLocalScale_t  g_SetLocalScale   = nullptr;  // Transform.set_localScale_Injected

// CinemachineBrain.LateUpdate hook (PostLateUpdate camera zoom — applied after Cinemachine
// commits its camera position, before Unity issues draw calls for the frame).
typedef void (__fastcall* fn_BrainLateUpdate_t)(void* brain, void* method);
static fn_BrainLateUpdate_t o_BrainLateUpdate = nullptr;

// Pending ability fires set by render thread, consumed by Unity main thread
static std::atomic<bool> s_pendingFireAb1{false};
static std::atomic<bool> s_pendingFireAb2{false};
static std::atomic<bool> s_pendingFireMod{false};
static bool                 s_brainLUHooked   = false;

// -- Runtime noclip state ---------------------------------------------
bool  Patches::g_NoclipActive  = false;
bool  Patches::g_UseCameraDir  = false;
float Patches::g_NoclipSpeed   = 800.0f;
float Patches::g_JumpPower     = 100.0f;
float Patches::g_BoostPower    = 100.0f;
int   Patches::g_NcKeyFwd      = 'W';
int   Patches::g_NcKeyBack     = 'S';
int   Patches::g_NcKeyRight    = 'D';
int   Patches::g_NcKeyLeft     = 'A';
int   Patches::g_NcKeyUp       = 'E';
int   Patches::g_NcKeyDown     = 'Q';
int   Patches::g_NcKeySprint   = VK_SHIFT;
static Vector3   g_NcPos       = {};
static void*     g_NcTransform = nullptr;
static void*     g_NcRigidbody = nullptr;
static void*     g_NcLastMech  = nullptr;
static void*     s_gotoEnemyTarget = nullptr;
static uint64_t  g_NcLastTick  = 0;

// -- One-time IL2CPP API resolution -----------------------------------
static bool InitNoclipAPI()
{
    HMODULE hGA = (HMODULE)GetBase();
    if (!hGA) return false;

    // Game function pointers via RVA offset from DLL base
    g_GetTransform     = (fn_GetTransform) (GetBase() + 0xA23D30);
    g_GetCompTransform = (fn_GetTransform) (GetBase() + 0x4DBF6E0);
    g_GetMainCamera    = (fn_GetMainCamera)(GetBase() + 0x4D87890);
    g_GetPosition      = (fn_GetVec3)      (GetBase() + 0x4DD3A40);
    g_GetForward       = (fn_GetVec3)      (GetBase() + 0x4DD34E0);
    g_GetRight         = (fn_GetVec3)      (GetBase() + 0x4DD3A90);
    g_SetPosition          = (fn_SetPosition)        (GetBase() + 0x4DD4290);
    g_SetVelocity          = (fn_SetVelocity)        (GetBase() + 0x4E50FD0);
    g_GetVelocityInjected  = (fn_GetVelocityInjected)(GetBase() + 0x4E506C0);
    g_HasEffect          = (fn_HasEffect)         (GetBase() + 0xB0B320);
    g_GetLocalScale        = (fn_GetVec3)            (GetBase() + 0x4DD3800); // get_localScale_Injected
    g_SetLocalScale    = (fn_SetLocalScale_t)(GetBase() + 0x4DD40E0); // set_localScale_Injected

    // IL2CPP API exported by GameAssembly.dll
    auto fnDomainGet     = (fn_domain_get)            GetProcAddress(hGA, "il2cpp_domain_get");
    auto fnDomainGetAsms = (fn_domain_get_assemblies) GetProcAddress(hGA, "il2cpp_domain_get_assemblies");
    auto fnAsmGetImage   = (fn_assembly_get_image)    GetProcAddress(hGA, "il2cpp_assembly_get_image");
    auto fnClassFromName = (fn_class_from_name)       GetProcAddress(hGA, "il2cpp_class_from_name");
    auto fnFieldFromName = (fn_field_from_name)       GetProcAddress(hGA, "il2cpp_class_get_field_from_name");
    g_FieldStaticGet     = (fn_field_static_get)      GetProcAddress(hGA, "il2cpp_field_static_get_value");

    if (!fnDomainGet || !fnDomainGetAsms || !fnAsmGetImage ||
        !fnClassFromName || !fnFieldFromName || !g_FieldStaticGet)
        return false;

    void* domain = fnDomainGet();
    if (!domain) return false;

    size_t asmCount = 0;
    void** assemblies = fnDomainGetAsms(domain, &asmCount);
    if (!assemblies || !asmCount) return false;

    for (size_t i = 0; i < asmCount; i++)
    {
        void* img = fnAsmGetImage(assemblies[i]);
        if (!img) continue;

        // MatchContext and PlayerContext both live in namespace "Game.Core", same assembly
        void* klass = fnClassFromName(img, "Game.Core", "MatchContext");
        if (klass)
        {
            void* fld = fnFieldFromName(klass, "MechManagement");
            if (fld) g_MechMgmtField = fld;
            void* fldNS = fnFieldFromName(klass, "NetworkService");
            if (fldNS) g_NetworkSvcField = fldNS;
        }

        void* pklass = fnClassFromName(img, "Game.Core", "PlayerContext");
        if (pklass)
        {
            void* fldLS = fnFieldFromName(pklass, "PlayerLoginService");
            if (fldLS) g_LoginSvcField = fldLS;
            void* fldSM = fnFieldFromName(pklass, "_squadManager");
            if (fldSM) g_SquadMgrField = fldSM;
            void* fldDM = fnFieldFromName(pklass, "DuelManager");
            if (fldDM) g_DuelMgrField = fldDM;
        }

        if (g_MechMgmtField) break;
    }

    return g_MechMgmtField != nullptr;
}

// -- Shared one-time API init guard -----------------------------------
static bool s_noclipAPIInited = false;
static bool s_noclipAPIFailed = false;
static bool EnsureNoclipAPI()
{
    if (!s_noclipAPIInited) { s_noclipAPIInited = true; s_noclipAPIFailed = !InitNoclipAPI(); }
    return !s_noclipAPIFailed;
}

// -- Pointer chain walker ---------------------------------------------
// Returns the active mech IL2CPP object pointer, or nullptr.
static void* GetActiveMech()
{
    if (!g_FieldStaticGet || !g_MechMgmtField) return nullptr;

    // MatchContext.MechManagement  (static field ? MechManagementService*)
    void* svc = nullptr;
    __try { g_FieldStaticGet(g_MechMgmtField, &svc); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!svc) return nullptr;

    // MechManagementService._playerMechState  (+0x30)
    void* playerState = nullptr;
    __try { playerState = *reinterpret_cast<void**>((char*)svc + 0x30); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!playerState) return nullptr;

    // PlayerMechState.IsMechActive  (+0x40)
    bool isActive = false;
    __try { isActive = *reinterpret_cast<bool*>((char*)playerState + 0x40); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!isActive) return nullptr;

    // PlayerMechState.ActiveMech  (+0x30)  ?  IActor*/Mech*
    void* mech = nullptr;
    __try { mech = *reinterpret_cast<void**>((char*)playerState + 0x30); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    return mech;
}

bool Patches::JumpUp()
{
    if (!EnsureNoclipAPI()) return false;
    if (!g_SetVelocity || !g_GetVelocityInjected) return false;

    void* mech = GetActiveMech();
    if (!mech) return false;

    void* rb = nullptr;
    __try { rb = *reinterpret_cast<void**>((char*)mech + 0xC8); }
    __except (EXCEPTION_EXECUTE_HANDLER) { rb = nullptr; }
    if (!rb) return false;

    Vector3 velocity = {};
    __try { g_GetVelocityInjected(rb, &velocity, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }

    velocity.y += Patches::g_JumpPower;
    __try { g_SetVelocity(rb, &velocity, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    return true;
}

bool Patches::BoostForward()
{
    if (!EnsureNoclipAPI()) return false;
    if (!g_SetVelocity || !g_GetVelocityInjected || !g_GetMainCamera || !g_GetCompTransform || !g_GetForward)
        return false;

    void* cam = nullptr;
    __try { cam = g_GetMainCamera(nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { cam = nullptr; }
    if (!cam) return false;

    void* camTf = nullptr;
    __try { camTf = g_GetCompTransform(cam, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { camTf = nullptr; }
    if (!camTf) return false;

    Vector3 forward = {};
    __try { g_GetForward(&forward, camTf, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }

    forward.y = 0.0f;
    float mag = sqrtf(forward.x * forward.x + forward.z * forward.z);
    if (mag <= 1e-5f) return false;
    forward.x /= mag;
    forward.z /= mag;

    void* mech = GetActiveMech();
    if (!mech) return false;

    void* rb = nullptr;
    __try { rb = *reinterpret_cast<void**>((char*)mech + 0xC8); }
    __except (EXCEPTION_EXECUTE_HANDLER) { rb = nullptr; }
    if (!rb) return false;

    Vector3 velocity = {};
    __try { g_GetVelocityInjected(rb, &velocity, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }

    velocity.x += forward.x * Patches::g_BoostPower;
    velocity.z += forward.z * Patches::g_BoostPower;
    __try { g_SetVelocity(rb, &velocity, nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    return true;
}

// -- Public tick (called every frame from hkPresent) ------------------
void Patches::NoclipTick()
{
    if (!g_NoclipActive) return;

    if (!EnsureNoclipAPI()) return;

    // When GotoEnemy is active it owns position — yield entirely so they don't fight.
    // Track the transition so we re-snapshot g_NcPos when GotoEnemy turns off,
    // preventing noclip from snapping back to its old pre-goto position.
    static bool s_gotoWasActive = false;
    if (g_AutoGotoEnemy)
    {
        s_gotoWasActive = true;
        return;
    }
    if (s_gotoWasActive)
    {
        s_gotoWasActive = false;
        if (g_NcTransform)
            __try { g_GetPosition(&g_NcPos, g_NcTransform, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    void* mech = GetActiveMech();
    if (!mech)
    {
        // Not in a match or mech is down � clear cached pointers
        g_NcTransform = nullptr;
        g_NcRigidbody = nullptr;
        g_NcLastMech  = nullptr;
        return;
    }

    // Detect mech change (respawn / hangar swap) — re-fetch Transform and snapshot position
    if (mech != g_NcLastMech)
    {
        g_NcLastMech  = mech;
        g_NcTransform = nullptr;
        g_NcRigidbody = nullptr;

        void* tf = nullptr;
        __try { tf = g_GetTransform(mech, nullptr); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        void* rb = nullptr;
        __try { rb = *reinterpret_cast<void**>((char*)mech + 0xC8); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        if (tf)
        {
            Vector3 cur{};
            __try { g_GetPosition(&cur, tf, nullptr); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
            g_NcPos = cur;
        }

        g_NcTransform = tf;
        g_NcRigidbody = rb;
        g_NcLastTick  = GetTickCount64();
        return; // skip movement on the first frame after mech change
    }

    if (!g_NcTransform) return;

    // Delta time (capped at 100 ms to ignore load spikes)
    uint64_t now = GetTickCount64();
    float dt = static_cast<float>(static_cast<double>(now - g_NcLastTick) / 1000.0);
    if (dt > 0.1f) dt = 0.1f;
    g_NcLastTick = now;

    // Sprint key doubles speed while held
    float speedMul = (g_NcKeySprint > 0 && (GetAsyncKeyState(g_NcKeySprint) & 0x8000)) ? 2.0f : 1.0f;
    const float spd = g_NoclipSpeed * speedMul * dt;
    Vector3 fwd = {0.f, 0.f, 1.f};   // world +Z
    Vector3 rgt = {1.f, 0.f, 0.f};   // world +X

    if (g_UseCameraDir && g_GetMainCamera && g_GetCompTransform && g_GetForward && g_GetRight)
    {
        void* cam = nullptr;
        __try { cam = g_GetMainCamera(nullptr); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        if (cam)
        {
            void* camTf = nullptr;
            __try { camTf = g_GetCompTransform(cam, nullptr); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}

            if (camTf)
            {
                __try { g_GetForward(&fwd, camTf, nullptr); }
                __except (EXCEPTION_EXECUTE_HANDLER) {}

                __try { g_GetRight(&rgt, camTf, nullptr); }
                __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        }
    }

    // Movement keys (default WASD + E up / Q down, all rebindable)
    if (g_NcKeyFwd   > 0 && (GetAsyncKeyState(g_NcKeyFwd)   & 0x8000)) { g_NcPos.x += fwd.x * spd; g_NcPos.y += fwd.y * spd; g_NcPos.z += fwd.z * spd; }
    if (g_NcKeyBack  > 0 && (GetAsyncKeyState(g_NcKeyBack)  & 0x8000)) { g_NcPos.x -= fwd.x * spd; g_NcPos.y -= fwd.y * spd; g_NcPos.z -= fwd.z * spd; }
    if (g_NcKeyRight > 0 && (GetAsyncKeyState(g_NcKeyRight) & 0x8000)) { g_NcPos.x += rgt.x * spd; g_NcPos.y += rgt.y * spd; g_NcPos.z += rgt.z * spd; }
    if (g_NcKeyLeft  > 0 && (GetAsyncKeyState(g_NcKeyLeft)  & 0x8000)) { g_NcPos.x -= rgt.x * spd; g_NcPos.y -= rgt.y * spd; g_NcPos.z -= rgt.z * spd; }
    if (g_NcKeyUp    > 0 && (GetAsyncKeyState(g_NcKeyUp)    & 0x8000)) g_NcPos.y += spd;
    if (g_NcKeyDown  > 0 && (GetAsyncKeyState(g_NcKeyDown)  & 0x8000)) g_NcPos.y -= spd;

    // Override position + zero velocity every frame to win against physics
    __try
    {
        if (g_NcRigidbody)
        {
            Vector3 zero{};
            g_SetVelocity(g_NcRigidbody, &zero, nullptr);
        }
        g_SetPosition(g_NcTransform, &g_NcPos, nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // Mech/transform was destroyed � invalidate cache so next frame re-fetches
        g_NcTransform = nullptr;
        g_NcRigidbody = nullptr;
        g_NcLastMech  = nullptr;
    }
}

// -- Enable / disable -------------------------------------------------
bool Patches::NoclipEnable(bool enable)
{
    g_NoclipActive = enable;
    if (!enable)
    {
        g_NcTransform = nullptr;
        g_NcRigidbody = nullptr;
        g_NcLastMech  = nullptr;
    }
    return true;
}

// -----------------------------------------------------------------------
// Huge Explosions � AoE radius stats-layer patch
//
// The explosion sphere radius is computed by WeaponAoeRadiusModifier.Modify()
// which reads EquipmentStats.get_AoeRadiusMultiplier and
// get_AoeRadiusAdditional, then pushes the result to the ECS component.
// Patching those two getters to return huge values forces every stats
// recalculation to produce a massive explosion sphere.
//
// Per-frame ECS setters (set_RadialDamage, set_maxRange) are called in
// GameplayTick() while g_HugeExplosions is true, giving every weapon
// splash damage and extending its firing range.
//
//   EquipmentStats.get_AoeRadiusMultiplier   RVA 0xE97AF0 ? 10000.0f
//   EquipmentStats.get_AoeRadiusAdditional   RVA 0xE97AD0 ? 10000.0f
// -----------------------------------------------------------------------
static const uintptr_t k_HugeExpRVAs[] = {
    0xF1A9B0,   // EquipmentStats.get_AoeRadiusMultiplier
    0xF1A990,   // EquipmentStats.get_AoeRadiusAdditional
};

// float getter (returns via xmm0 in x64) — always returns 10000.0f.
static float __fastcall hkRetHuge() { return 10000.0f; }
static bool s_hugeExpHooksCreated = false;

bool Patches::HugeExplosions(bool enable)
{
    if (!GetBase()) return false;

    if (!s_hugeExpHooksCreated)
    {
        int created = 0;
        for (uintptr_t rva : k_HugeExpRVAs)
        {
            void* target = reinterpret_cast<void*>(A(rva));
            void* orig   = nullptr;
            if (MH_CreateHook(target, reinterpret_cast<void*>(&hkRetHuge), &orig) != MH_OK)
            {
                for (int r = 0; r < created; r++)
                    MH_RemoveHook(reinterpret_cast<void*>(A(k_HugeExpRVAs[r])));
                return false;
            }
            ++created;
        }
        s_hugeExpHooksCreated = true;
    }

    for (uintptr_t rva : k_HugeExpRVAs)
    {
        void* target = reinterpret_cast<void*>(A(rva));
        if (enable) MH_QueueEnableHook(target);
        else        MH_QueueDisableHook(target);
    }
    MH_ApplyQueued();
    return true;
}

// -----------------------------------------------------------------------
// Damage Boost — per-frame weapon field write + CreateHitEventData hook
//
// Two-layer approach so the boosted value reaches the server:
//   1. Weapon._internalDamageMultiplier (+0xA4) written every frame.
//      WeaponInternalDamageMultiplierModifier reads this field and pushes
//      it into DamageInfo.DamageMultiplier (+0x34).
//   2. CreateHitEventData (RVA 0x131C610) is hooked: after the original
//      copies DamageInfo.DamageMultiplier into LocalHitEvent.DamageMultiplier
//      (+0x18), the hook multiplies it again — ensuring the value leaving
//      in the hit packet is fully boosted.
// -----------------------------------------------------------------------
typedef bool (__fastcall* fn_CreateHitEventData_t)(void*, void*, void*, void*, void*);
static fn_CreateHitEventData_t orig_CreateHitEventData = nullptr;
static bool s_dmgBoostHookCreated = false;

static bool __fastcall hk_CreateHitEventData(void* equip, void* actor, void* hitCtrl, void* damageInfo, void* outHitEvent)
{
    bool result = orig_CreateHitEventData(equip, actor, hitCtrl, damageInfo, outHitEvent);
    if (result && outHitEvent && Patches::g_DamageBoost)
    {
        __try { *(float*)((char*)outHitEvent + 0x18) *= Patches::g_DamageBoostMul; }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return result;
}

bool Patches::DamageBoostHook(bool enable)
{
    if (!GetBase()) return false;
    if (!s_dmgBoostHookCreated)
    {
        void* target = reinterpret_cast<void*>(A(0x131C610));
        if (MH_CreateHook(target, reinterpret_cast<void*>(&hk_CreateHitEventData),
                          reinterpret_cast<void**>(&orig_CreateHitEventData)) != MH_OK)
            return false;
        s_dmgBoostHookCreated = true;
    }
    void* target = reinterpret_cast<void*>(A(0x131C610));
    if (enable) MH_QueueEnableHook(target);
    else        MH_QueueDisableHook(target);
    MH_ApplyQueued();
    return true;
}

// -----------------------------------------------------------------------
// Combat features � Rapid Fire, Infinite Ammo,
//                   Instant Cooldown, Duration Override, Auto-use
//
// Weapon path:
//   mech +0x258 ? ShootingSlotCollectionManager
//     +0x18 ? ShootingSlotManager[] (IL2CPP array)
//       [i] ? ShootingSlotManager
//         +0x10 ? List<IShootingWeapon>
//           list+0x10 ? IShootingWeapon[] items array (IL2CPP)
//             items+0x20+j*8 ? Weapon*
//               +0x14C : _ammoCapacity (float)
//               +0x154 : _ammoValue    (float)  � current ammo
//               +0x158 : _shotInterval  (float)
//
// Ability / Module path:
//   AbilityHudController.get_Instance()  RVA 0x12E7600 ? AHC*
//     +0x40 ? NetworkAbilityCustomizable[] _abilities  (idx 0=ab1, 1=ab2)
//     +0x48 ? NetworkAbilityCustomizable[] _modules    (idx 0=module)
//   NetworkAbilityCustomizable (NAC):
//     +0x60 ? AbilityState[] _states
//       state[1]       : AbilityStateWithDuration   � _duration at +0x68
//       state[count-1] : AbilityStateWithCooldown   � _cooldown at +0x70
//   NAC.GotoNextState()  RVA 0xFDF190
// -----------------------------------------------------------------------

typedef void* (__fastcall* fn_GetAHCInst)           (void* method);
typedef void  (__fastcall* fn_GotoNextState_t)       (void* ability, void* method);
// Weapon.IShootingWeapon.ScheduleShots(int count)          RVA 0x97A510
typedef void  (__fastcall* fn_ScheduleShots_t)       (void* weapon, int count, void* method);
// Weapon.IShootingWeapon.set_ShotInterval(float)           RVA 0x97AD30
typedef void  (__fastcall* fn_SetShotInterval_t)     (void* weapon, float value, void* method);
// Weapon.IShootingWeapon.set_AmmoConsumptionPerShot(float) RVA 0x97A7D0
typedef void  (__fastcall* fn_SetAmmoConsumption_t)  (void* weapon, float value, void* method);
static fn_GetAHCInst          g_GetAHCInstance    = nullptr;
static fn_GotoNextState_t     g_GotoNextState     = nullptr;
static fn_ScheduleShots_t     g_ScheduleShots     = nullptr;
static fn_SetShotInterval_t   g_SetShotInterval   = nullptr;
static fn_SetAmmoConsumption_t g_SetAmmoConsumption = nullptr;

// Runtime flags
int   Patches::g_FPSOverride    = 0;
bool  Patches::g_RapidFire      = false;
bool  Patches::g_InfiniteAmmo   = false;
bool  Patches::g_HugeExplosions = false;
bool  Patches::g_AutoGotoEnemy    = false;
bool  Patches::g_IgnoreAbsorber   = false;
bool  Patches::g_IgnorePhaseShift = false;
bool  Patches::g_IgnoreCounterShield = false;
bool  Patches::g_AntiStealth = false;
bool  Patches::g_AntiEMP = false;
bool  Patches::g_RotationSpeed5x = false;
bool  Patches::g_InstantTargetLockSpeed = false;
bool  Patches::g_AutoClaimRewards = false;
bool  Patches::g_AutoFire         = false;
bool  Patches::g_DamageBoost      = false;
float Patches::g_DamageBoostMul   = 10.0f;
float Patches::g_GotoEnemyOffsetX = 0.0f;
float Patches::g_GotoEnemyOffsetY = 0.0f;
float Patches::g_GotoEnemyOffsetZ = 0.0f;
bool  Patches::g_GhostGotoEnemy   = false;
bool  Patches::g_Invisible        = false;
bool  Patches::g_ForceGrounded    = false;
bool  Patches::g_Instant1CD     = false;
bool  Patches::g_Instant2CD     = false;
bool  Patches::g_InstantModCD   = false;
bool  Patches::g_Ability1Dur    = false;
bool  Patches::g_Ability2Dur    = false;
bool  Patches::g_ModuleDur      = false;
float Patches::g_Ability1DurVal = 9999.0f;
float Patches::g_Ability2DurVal = 9999.0f;
float Patches::g_ModuleDurVal   = 9999.0f;
bool  Patches::g_AutoAbility1   = false;
bool  Patches::g_AutoAbility2   = false;
bool  Patches::g_AutoModule     = false;
float Patches::g_AutoA1Interval = 1.0f;
float Patches::g_AutoA2Interval = 1.0f;
float Patches::g_AutoModInterval= 1.0f;
bool  Patches::g_HidePlayerName    = false;
float Patches::g_PlayerScaleSlider  = 0.0f;
float Patches::g_CameraFOV          = 0.0f;
float Patches::g_CameraZoom         = 0.0f;

static uint64_t g_AutoLastTick[3] = {};   // [0]=ab1 [1]=ab2 [2]=mod


// -- Player name obfuscation ---------------------------------------------
// Write our fake name via Player.set_name() on enable and restore on disable.
// GameplayTick also re-applies it every frame while enabled so any server
// reset that overwrites the static field is overridden immediately.
static char s_savedPlayerName[128] = {};
static bool s_hideNameActive       = false;

bool Patches::HidePlayerName(bool enable)
{
    if (!GetBase()) return false;
    fn_SetPlayerName_t fnSetName = (fn_SetPlayerName_t)(GetBase() + 0xEF2790); // Player.set_name
    fn_Il2cppStrNew_t  fnStrNew  = (fn_Il2cppStrNew_t)GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");
    if (!fnSetName || !fnStrNew) return false;

    if (enable)
    {
        if (s_hideNameActive) return true;
        // Snapshot the current player name
        fn_GetPlayerName_t fnGetName = (fn_GetPlayerName_t)(GetBase() + 0xEF1DA0);
        void* strObj = nullptr;
        __try { strObj = fnGetName(nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
        memset(s_savedPlayerName, 0, sizeof(s_savedPlayerName));
        if (strObj)
        {
            int len = 0;
            __try { len = *(int*)((char*)strObj + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}
            for (int i = 0; i < len && i < 127; i++)
            {
                __try {
                    wchar_t wc = *(wchar_t*)((char*)strObj + 0x14 + i * 2);
                    s_savedPlayerName[i] = (char)(wc & 0x7F);
                } __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            }
        }
        void* fakeStr = nullptr;
        __try { fakeStr = fnStrNew("<a>TitanCore</a>"); } __except (EXCEPTION_EXECUTE_HANDLER) {}
        if (!fakeStr) return false;
        __try { fnSetName(fakeStr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
        s_hideNameActive = true;
        g_HidePlayerName = true;
    }
    else
    {
        if (!s_hideNameActive) return true;
        void* origStr = nullptr;
        __try { origStr = fnStrNew(s_savedPlayerName[0] ? s_savedPlayerName : "Player"); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
        if (origStr)
            __try { fnSetName(origStr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
        s_hideNameActive = false;
        g_HidePlayerName = false;
    }
    return true;
}
// -----------------------------------------------------------------------
// GetState hook — shared by Ghost Goto Enemy and Invisible
//   RVA: 0x76F4000  (NetMechDynamics.GetState)
//
//   GetState fills the outgoing NetDynamics model each network tick.
//   We overwrite NetDynamics.PositionV2 (Float3Position @ +0x0) after the
//   original runs so the delta serializer encodes our spoofed position.
//   Local Transform is never touched — the client sees the real location.
//
//   Float3Position layout: X@+0x0, Y@+0x4, Z@+0x8
//   StateTime = single uint32 (fits in register)
//
//   IL2CPP x64:
//     void __fastcall GetState(this, NetDynamics* model, uint32 serverTime, uint32 ownerTime, MethodInfo*)
// -----------------------------------------------------------------------
typedef void(__fastcall* fn_GetState_t)(void* self, void* model, uint32_t serverTime, uint32_t ownerTime, void* method);
static fn_GetState_t o_GetState           = nullptr;
static bool          s_getStateHookCreated = false;
static Vector3       s_ghostGotoTargetPos  = {};
static bool          s_ghostGotoHasTarget  = false;

static void __fastcall hk_GetState(void* self, void* model, uint32_t serverTime, uint32_t ownerTime, void* method)
{
    o_GetState(self, model, serverTime, ownerTime, method);
    if (!model) return;

    if (Patches::g_GhostGotoEnemy && s_ghostGotoHasTarget)
    {
        // Spoof position to the resolved enemy location
        __try
        {
            *(float*)((char*)model + 0x0) = s_ghostGotoTargetPos.x;
            *(float*)((char*)model + 0x4) = s_ghostGotoTargetPos.y;
            *(float*)((char*)model + 0x8) = s_ghostGotoTargetPos.z;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    else if (Patches::g_Invisible)
    {
        // Push Z astronomically far so the server places us off-map
        __try
        {
            float z = *(float*)((char*)model + 0x8);
            *(float*)((char*)model + 0x8) = z * 1E+32f;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
}

// Installs the GetState hook once and enables/disables it based on whether
// either Ghost Goto Enemy or Invisible is currently active.
static bool EnsureGetStateHook()
{
    if (s_getStateHookCreated) return true;
    void* target = reinterpret_cast<void*>(A(0x77D4130));
    MH_STATUS st = MH_CreateHook(target,
                      reinterpret_cast<void*>(&hk_GetState),
                      reinterpret_cast<void**>(&o_GetState));
    PLog("EnsureGetStateHook MH_CreateHook target=%p status=%d", target, (int)st);
    if (st != MH_OK) return false;
    s_getStateHookCreated = true;
    return true;
}

static void UpdateGetStateHook()
{
    if (!s_getStateHookCreated) return;
    void* target = reinterpret_cast<void*>(A(0x77D4130));
    bool needed = Patches::g_GhostGotoEnemy || Patches::g_Invisible;
    if (needed) MH_QueueEnableHook(target);
    else        MH_QueueDisableHook(target);
    MH_ApplyQueued();
}

bool Patches::GhostGotoEnemyHook(bool enable)
{
    if (!GetBase()) return false;
    if (!EnsureGetStateHook()) return false;
    if (!enable) s_ghostGotoHasTarget = false;
    UpdateGetStateHook();
    PLog("GhostGotoEnemyHook %s", enable ? "enabled" : "disabled");
    return true;
}

bool Patches::InvisibleHook(bool enable)
{
    if (!GetBase()) return false;
    if (!EnsureGetStateHook()) return false;
    UpdateGetStateHook();
    PLog("InvisibleHook %s", enable ? "enabled" : "disabled");
    return true;
}

// -----------------------------------------------------------------------
// Force IsGrounded
// Hook a client-side IsGrounded() method and make it always return true.
// RVA provided: 0x87C490
// -----------------------------------------------------------------------
static bool s_forceGroundHookCreated = false;
static bool (__fastcall* o_IsGrounded)(void* self, void* method) = nullptr;
static bool __fastcall hk_IsGrounded(void* self, void* method)
{
    // Always report grounded
    return true;
}

static bool EnsureIsGroundedHook()
{
    if (s_forceGroundHookCreated) return true;
    void* target = reinterpret_cast<void*>(A(0x881DC0));
    MH_STATUS st = MH_CreateHook(target,
                      reinterpret_cast<void*>(&hk_IsGrounded),
                      reinterpret_cast<void**>(&o_IsGrounded));
    PLog("EnsureIsGroundedHook MH_CreateHook target=%p status=%d", target, (int)st);
    if (st != MH_OK) return false;
    s_forceGroundHookCreated = true;
    return true;
}

static void UpdateIsGroundedHook()
{
    if (!s_forceGroundHookCreated) return;
    void* target = reinterpret_cast<void*>(A(0x881DC0));
    if (Patches::g_ForceGrounded) MH_QueueEnableHook(target);
    else                             MH_QueueDisableHook(target);
    MH_ApplyQueued();
}

bool Patches::ForceGroundedHook(bool enable)
{
    if (!GetBase()) return false;
    if (!EnsureIsGroundedHook()) return false;
    UpdateIsGroundedHook();
    PLog("ForceGroundedHook %s", enable ? "enabled" : "disabled");
    return true;
}

static bool InitGameplayAPI()
{
    if (!GetBase()) return false;
    g_GetAHCInstance  = (fn_GetAHCInst)        (GetBase() + 0x12E7600);
    g_GotoNextState   = (fn_GotoNextState_t)   (GetBase() + 0xFDF190);
    g_ScheduleShots      = (fn_ScheduleShots_t)      (GetBase() + 0x97A510);
    g_SetShotInterval    = (fn_SetShotInterval_t)    (GetBase() + 0x97AD30);
    g_SetAmmoConsumption = (fn_SetAmmoConsumption_t) (GetBase() + 0x97A7D0);
    return true;
}

// -- Ability object accessors ------------------------------------------
// Returns NAC* for ability at [idx] in AHC._abilities, or nullptr.
static void* GetAbilityObj(int idx)
{
    if (!g_GetAHCInstance) return nullptr;
    void* ahc = nullptr;
    __try { ahc = g_GetAHCInstance(nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!ahc) return nullptr;

    void* arr = nullptr;
    __try { arr = *(void**)((char*)ahc + 0x40); }   // _abilities[]
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!arr) return nullptr;

    int count = 0;
    __try { count = *(int*)((char*)arr + 0x18); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (idx < 0 || idx >= count) return nullptr;

    void* obj = nullptr;
    __try { obj = *(void**)((char*)arr + 0x20 + idx * 8); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return obj;
}

// Returns NAC* for module at [idx] in AHC._modules, or nullptr.
static void* GetModuleObj(int idx)
{
    if (!g_GetAHCInstance) return nullptr;
    void* ahc = nullptr;
    __try { ahc = g_GetAHCInstance(nullptr); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!ahc) return nullptr;

    void* arr = nullptr;
    __try { arr = *(void**)((char*)ahc + 0x48); }   // _modules[]
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (!arr) return nullptr;

    int count = 0;
    __try { count = *(int*)((char*)arr + 0x18); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
    if (idx < 0 || idx >= count) return nullptr;

    void* obj = nullptr;
    __try { obj = *(void**)((char*)arr + 0x20 + idx * 8); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    return obj;
}

// -- Ability state helpers ---------------------------------------------
// Set _cooldown on the last state in NAC._states to 0.01f (instant).
static void ApplyInstantCD(void* nac)
{
    void* statesArr = nullptr;
    __try { statesArr = *(void**)((char*)nac + 0x60); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (!statesArr) return;

    int count = 0;
    __try { count = *(int*)((char*)statesArr + 0x18); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (count < 2 || count > 16) return;

    void* cdState = nullptr;
    __try { cdState = *(void**)((char*)statesArr + 0x20 + (count - 1) * 8); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (!cdState) return;

    __try { *(float*)((char*)cdState + 0x70) = 0.01f; }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// Set _duration on state[1] in NAC._states to `dur` seconds.
// Also overwrite _endDurationTime = _activateTime + dur every frame so the
// game's own timer is continuously reset to our desired value.
static void ApplyDuration(void* nac, float dur)
{
    void* statesArr = nullptr;
    __try { statesArr = *(void**)((char*)nac + 0x60); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (!statesArr) return;

    int count = 0;
    __try { count = *(int*)((char*)statesArr + 0x18); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (count < 2 || count > 16) return;

    void* actState = nullptr;
    __try { actState = *(void**)((char*)statesArr + 0x20 + 1 * 8); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (!actState) return;

    __try {
        // _duration (+0x68): config value read by game when entering this state
        *(float*)((char*)actState + 0x68) = dur;
        // _activateTime (+0x48): set by game when the state is entered
        float activateTime = *(float*)((char*)actState + 0x48);
        // _endDurationTime (+0x70): actual expiry = activateTime + _duration
        // Write it every frame so the game can't expire the ability
        *(float*)((char*)actState + 0x70) = activateTime + dur;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// Hook postfix for CinemachineBrain.LateUpdate.
// Call original first so Cinemachine finishes writing its final camera position,
// then scale that position away from the mech by g_CameraZoom.
static void __fastcall hkBrainLateUpdate(void* brain, void* method)
{
    if (o_BrainLateUpdate) o_BrainLateUpdate(brain, method);

    // Consume pending ability fires from the render thread.
    // We're on the Unity main thread here, so it's safe to call managed methods.
    if (s_pendingFireAb1 || s_pendingFireAb2 || s_pendingFireMod)
    {
        void* ab1p = s_pendingFireAb1 ? GetAbilityObj(0) : nullptr;
        void* ab2p = s_pendingFireAb2 ? GetAbilityObj(1) : nullptr;
        void* modp = s_pendingFireMod  ? GetModuleObj(0)  : nullptr;

        if (s_pendingFireAb1) { s_pendingFireAb1 = false; if (ab1p) { __try { g_GotoNextState(ab1p, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {} } }
        if (s_pendingFireAb2) { s_pendingFireAb2 = false; if (ab2p) { __try { g_GotoNextState(ab2p, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {} } }
        if (s_pendingFireMod) { s_pendingFireMod = false; if (modp) { __try { g_GotoNextState(modp, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {} } }
    }

    if (Patches::g_CameraZoom == 0.0f) return;
    if (!g_GetMainCamera || !g_GetCompTransform || !g_GetPosition || !g_SetPosition) return;

    void* zMech = GetActiveMech();
    if (!zMech) return;

    void* zCam = nullptr;
    __try { zCam = g_GetMainCamera(nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (!zCam) return;

    void* zCamTf = nullptr, *zMechTf = nullptr;
    __try { zCamTf = g_GetCompTransform(zCam, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    __try { zMechTf = g_GetTransform(zMech, nullptr); }  __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (!zCamTf || !zMechTf) return;

    Vector3 zCamPos{}, zMechPos{};
    __try { g_GetPosition(&zCamPos, zCamTf, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    __try { g_GetPosition(&zMechPos, zMechTf, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}

    // Only scale the horizontal (XZ) offset so the camera moves back, not up.
    // Y stays at the original Cinemachine height to avoid the camera drifting skyward.
    Vector3 zNewPos = {
        zMechPos.x + (zCamPos.x - zMechPos.x) * Patches::g_CameraZoom,
        zCamPos.y,
        zMechPos.z + (zCamPos.z - zMechPos.z) * Patches::g_CameraZoom
    };
    __try { g_SetPosition(zCamTf, &zNewPos, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// -- Main per-frame tick -----------------------------------------------
void Patches::GameplayTick()
{
    // Attach the render thread to the IL2CPP runtime once so the GC knows
    // about it.  Without this, any GC collection triggered while we call
    // managed functions from the Present hook crashes with "unknown thread".
    static bool s_threadAttached = false;
    if (!s_threadAttached)
    {
        HMODULE hGA = (HMODULE)GetBase();
        typedef void* (*fn_domain_get_t)();
        typedef void* (*fn_thread_attach_t)(void* domain);
        auto fnDomainGet    = (fn_domain_get_t)   GetProcAddress(hGA, "il2cpp_domain_get");
        auto fnThreadAttach = (fn_thread_attach_t)GetProcAddress(hGA, "il2cpp_thread_attach");
        if (fnDomainGet && fnThreadAttach)
        {
            void* domain = fnDomainGet();
            if (domain) fnThreadAttach(domain);
        }
        s_threadAttached = true;  // attempt only once regardless of outcome
    }

    // Uncap FPS � Application.set_targetFrameRate(-1) called every frame
    // RVA 0x4D84BF0 : static void Application.set_targetFrameRate(int value)
    if (GetBase())
    {
        static int s_lastFPS = -999;
        int fps = (g_FPSOverride == 0) ? 30 : g_FPSOverride;
        if (fps != s_lastFPS)
        {
            auto fnSetFPS = (void(__fastcall*)(int, void*))(GetBase() + 0x4D84BF0);
            __try { fnSetFPS(fps, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
            s_lastFPS = fps;
        }
    }

    const bool anyWeapon  = g_RapidFire || g_InfiniteAmmo || g_AutoFire || g_DamageBoost;
    const bool anyAbility = g_Instant1CD || g_Instant2CD || g_InstantModCD
                          || g_Ability1Dur || g_Ability2Dur || g_ModuleDur
                          || g_AutoAbility1 || g_AutoAbility2 || g_AutoModule;
    static bool s_fovWasActive = false;
    const bool anyPlayer  = g_HidePlayerName || g_PlayerScaleSlider != 0.0f || g_CameraFOV > 0.0f || s_fovWasActive || g_CameraZoom != 0.0f;
    const bool anyAutoClaim = g_AutoClaimRewards;

    if (anyAutoClaim)
    {
        static uint64_t s_lastAutoClaim = 0;
        uint64_t now = GetTickCount64();
        if (now - s_lastAutoClaim >= 10000)
        {
            s_lastAutoClaim = now;
            ClaimAllQuests();
        }
    }

    if (!anyWeapon && !anyAbility && !g_AutoGotoEnemy && !g_GhostGotoEnemy && !anyPlayer && !anyAutoClaim) return;

    // One-time lightweight init (just function pointer resolution via RVA)
    static bool s_inited = false, s_failed = false;
    if (!s_inited) { s_inited = true; s_failed = !InitGameplayAPI(); }
    if (s_failed) return;

    // -- Weapons -------------------------------------------------------
    if (anyWeapon)
    {
        // Weapon walk requires the IL2CPP chain (reuse NoclipTick's init lazy-path).
        // GetActiveMech() returns null if noclip API isn't inited yet; we do a
        // direct static field read which is only valid after InitNoclipAPI() ran.
        // So if noclip was never enabled, we bootstrap the API here lazily.
        EnsureNoclipAPI();
        void* mech = GetActiveMech();
        static bool s_mechLogged = false;
        if (!s_mechLogged) { s_mechLogged = true; PLog("GetActiveMech -> %p", mech); }
        if (mech)
        {
            // mech._shootingSlotCollectionManager at +0x258
            void* sscMgr = nullptr;
            __try { sscMgr = *(void**)((char*)mech + 0x258); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}

            void* ssArr = nullptr;
            if (sscMgr) { __try { ssArr = *(void**)((char*)sscMgr + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {} }

            int slotCount = 0;
            if (ssArr) { __try { slotCount = *(int*)((char*)ssArr + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {} }

            for (int i = 0; i < slotCount && i < 16; i++)
            {
                void* slotMgr = nullptr;
                __try { slotMgr = *(void**)((char*)ssArr + 0x20 + i * 8); }
                __except (EXCEPTION_EXECUTE_HANDLER) {}
                if (!slotMgr) continue;

                void* wList = nullptr;
                __try { wList = *(void**)((char*)slotMgr + 0x10); }
                __except (EXCEPTION_EXECUTE_HANDLER) {}
                if (!wList) continue;

                void* items = nullptr;
                int   sz    = 0;
                __try { items = *(void**)((char*)wList + 0x10); sz = *(int*)((char*)wList + 0x18); }
                __except (EXCEPTION_EXECUTE_HANDLER) {}
                if (!items || sz <= 0 || sz > 64) continue;

                for (int j = 0; j < sz; j++)
                {
                    void* weapon = nullptr;
                    __try { weapon = *(void**)((char*)items + 0x20 + j * 8); }
                    __except (EXCEPTION_EXECUTE_HANDLER) {}
                    if (!weapon) continue;

                    if (g_RapidFire)
                    {
                        if (g_SetShotInterval)
                            __try { g_SetShotInterval(weapon, 0.017f, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    }

                    if (g_AutoFire)
                    {
                        if (g_ScheduleShots)
                            __try { g_ScheduleShots(weapon, 5, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    }

                    if (g_InfiniteAmmo)
                    {
                        // Zero consumption via ECS-aware setter � this is what actually works
                        if (g_SetAmmoConsumption)
                            __try { g_SetAmmoConsumption(weapon, 0.0f, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    }

                    if (g_DamageBoost)
                    {
                        // Write _internalDamageMultiplier directly; the modifier system
                        // reads this field and propagates it into every outgoing hit packet.
                        __try { *(float*)((char*)weapon + 0xA4) = g_DamageBoostMul; }
                        __except (EXCEPTION_EXECUTE_HANDLER) {}
                    }
                }
            }
        }
    }

    // -- Abilities & Module --------------------------------------------
    void* ab1 = nullptr;
    void* ab2 = nullptr;
    void* mod = nullptr;

    // -- Auto-use (GotoNextState on configurable timer) ----------------
    uint64_t now = GetTickCount64();

    // Respawn / game-end guard:
    // Fire a 5-second grace period on EITHER transition:
    //   non-null ? null  (death or game ending / loading screen)
    //   null     ? non-null  (respawn)
    // During the grace window all ability calls are suppressed to avoid
    // calling GotoNextState / ApplyInstantCD on a torn-down AHC/NAC.
    static void*    s_autoGuardMech  = (void*)1;  // non-null sentinel avoids first-frame trigger
    static uint64_t s_autoGraceUntil = 0;
    {
        void* curMech = GetActiveMech();
        bool wasAlive = (s_autoGuardMech != nullptr);
        bool isAlive  = (curMech != nullptr);
        if (wasAlive != isAlive)   // any state change ? start grace
        {
            g_AutoLastTick[0] = g_AutoLastTick[1] = g_AutoLastTick[2] = now;
            s_autoGraceUntil  = now + 1000;
        }
        s_autoGuardMech = curMech;
    }
    const bool autoOk = (now >= s_autoGraceUntil);

    // Only fetch/use ability objects at all when:
    //   a) mech is valid (not during loading screen / after game end)
    //   b) grace period has elapsed
    //   c) AHC._currentActor (+0x38) is non-null  => ability HUD is actively showing
    void* ahcForGuard = nullptr;
    if (g_GetAHCInstance) __try { ahcForGuard = g_GetAHCInstance(nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    bool ahcHasActor = false;
    if (ahcForGuard) __try { ahcHasActor = (*reinterpret_cast<void**>((char*)ahcForGuard + 0x38) != nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (anyAbility && s_autoGuardMech && autoOk && ahcHasActor)
    {
        ab1 = GetAbilityObj(0);
        ab2 = GetAbilityObj(1);
        mod = GetModuleObj(0);

        // Determine which slots will fire GotoNextState this frame.
        // ApplyInstantCD / ApplyDuration are skipped for those slots so the
        // state machine is never written to AND transitioned in the same frame.
        bool fireAb1 = g_AutoAbility1 && ab1 && (now - g_AutoLastTick[0] >= (uint64_t)(g_AutoA1Interval  * 1000.f));
        bool fireAb2 = g_AutoAbility2 && ab2 && (now - g_AutoLastTick[1] >= (uint64_t)(g_AutoA2Interval  * 1000.f));
        bool fireMod = g_AutoModule   && mod && (now - g_AutoLastTick[2] >= (uint64_t)(g_AutoModInterval * 1000.f));

        // ApplyInstantCD / ApplyDuration are also managed calls — do them
        // only on non-firing frames and defer to the Unity main thread via
        // hkBrainLateUpdate.  For the instant-CD/duration-only path we call
        // them directly here while hkBrainLateUpdate is not installed yet;
        // once the hook is up these execute on the main thread already.
        if (g_Instant1CD   && ab1 && !fireAb1) ApplyInstantCD(ab1);
        if (g_Instant2CD   && ab2 && !fireAb2) ApplyInstantCD(ab2);
        if (g_InstantModCD && mod && !fireMod)  ApplyInstantCD(mod);

        if (g_Ability1Dur && ab1 && !fireAb1) ApplyDuration(ab1, g_Ability1DurVal);
        if (g_Ability2Dur && ab2 && !fireAb2) ApplyDuration(ab2, g_Ability2DurVal);
        if (g_ModuleDur   && mod && !fireMod)  ApplyDuration(mod,  g_ModuleDurVal);

        // Set pending-fire flags — the actual GotoNextState calls happen
        // in hkBrainLateUpdate on the Unity main thread to avoid the GC
        // racing with our render-thread execution at high frame rates.
        if (fireAb1) { s_pendingFireAb1 = true; g_AutoLastTick[0] = now; }
        if (fireAb2) { s_pendingFireAb2 = true; g_AutoLastTick[1] = now; }
        if (fireMod) { s_pendingFireMod = true;  g_AutoLastTick[2] = now; }
    }

    // -- Auto Goto Weakest Enemy ---------------------------------------
    if (g_AutoGotoEnemy || g_GhostGotoEnemy)
    {
        if (EnsureNoclipAPI())
        {
            void* localMech = GetActiveMech();
            if (localMech)
            {
                // Walk: MechManagementService._mechRegistry (+0x38) ? MechRegistry
                void* svc = nullptr;
                __try { g_FieldStaticGet(g_MechMgmtField, &svc); } __except(EXCEPTION_EXECUTE_HANDLER){}
                void* registry = nullptr;
                if (svc) __try { registry = *(void**)((char*)svc + 0x38); } __except(EXCEPTION_EXECUTE_HANDLER){}

                // Read local mech's team pointer for enemy filtering
                // BaseCombatActor.<Team>k__BackingField at +0xA0
                void* localTeam = nullptr;
                __try { localTeam = *(void**)((char*)localMech + 0xA0); } __except(EXCEPTION_EXECUTE_HANDLER){}

                // MechRegistry._mechs (+0x10) ? Dictionary<ViewId,IActor>
                void* dict = nullptr;
                if (registry) __try { dict = *(void**)((char*)registry + 0x10); } __except(EXCEPTION_EXECUTE_HANDLER){}

                // IL2CPP Dictionary<ViewId,IActor> entries:
                //   dict+0x18 ? Entry[] array
                //   dict+0x20 ? int count (used slots)
                //   Entry layout (24 bytes): int hash | int next | uint ViewId | pad4 | IActor* (8)
                void* entriesArr = nullptr;
                int   dictCount  = 0;
                if (dict)
                {
                    __try { entriesArr = *(void**)((char*)dict + 0x18); } __except(EXCEPTION_EXECUTE_HANDLER){}
                    __try { dictCount  = *(int* )((char*)dict + 0x20); } __except(EXCEPTION_EXECUTE_HANDLER){}
                }

                // Keep the current enemy target until it is dead/gone or becomes invalid.
                void* currentTarget = s_gotoEnemyTarget;
                if (currentTarget)
                {
                    bool keepTarget = true;
                    if (currentTarget == localMech)
                        keepTarget = false;
                    else
                    {
                        void* actorTeam = nullptr;
                        __try { actorTeam = *(void**)((char*)currentTarget + 0xA0); } __except(EXCEPTION_EXECUTE_HANDLER){ actorTeam = nullptr; }
                        if (actorTeam == localTeam)
                            keepTarget = false;
                        else
                        {
                            void* hc = nullptr;
                            float hp = 0.f;
                            __try { hc = *(void**)((char*)currentTarget + 0x78); } __except(EXCEPTION_EXECUTE_HANDLER){ hc = nullptr; }
                            if (!hc)
                                keepTarget = false;
                            else
                            {
                                __try { hp = *(float*)((char*)hc + 0x18); } __except(EXCEPTION_EXECUTE_HANDLER){ hp = 0.f; }
                                if (hp <= 0.f)
                                    keepTarget = false;
                            }
                        }
                    }

                    if (keepTarget && g_HasEffect)
                    {
                        if (g_IgnoreAbsorber && g_HasEffect(currentTarget, 76, nullptr)) keepTarget = false;
                        if (g_IgnorePhaseShift && g_HasEffect(currentTarget, 42, nullptr)) keepTarget = false;
                        if (g_IgnoreCounterShield && g_HasEffect(currentTarget, 70, nullptr)) keepTarget = false;
                    }

                    if (!keepTarget)
                        currentTarget = s_gotoEnemyTarget = nullptr;
                }

                Vector3 reflectorPos[64];
                int reflectorCount = 0;
                for (int ei = 0; entriesArr && ei < dictCount && ei < 64; ei++)
                {
                    char* entry = (char*)entriesArr + 0x20 + ei * 24;
                    int   hash  = 0;
                    __try { hash = *(int*)entry; } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                    if (hash < 0) continue;

                    void* actor = nullptr;
                    __try { actor = *(void**)(entry + 16); } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                    if (!actor) continue;

                    if (g_HasEffect && g_HasEffect(actor, 70, nullptr))
                    {
                        void* xfm = nullptr;
                        __try { xfm = g_GetTransform(actor, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){ xfm = nullptr; }
                        if (xfm && reflectorCount < 64)
                        {
                            __try { g_GetPosition(&reflectorPos[reflectorCount], xfm, nullptr); }
                            __except(EXCEPTION_EXECUTE_HANDLER) { continue; }
                            reflectorCount++;
                        }
                    }
                }

                void* bestEnemy    = nullptr;
                float bestHp       = 1e30f;
                float bestMinDistSq = -1.0f;

                if (!currentTarget)
                {
                    for (int ei = 0; entriesArr && ei < dictCount && ei < 64; ei++)
                    {
                        char* entry = (char*)entriesArr + 0x20 + ei * 24;
                        int   hash  = 0;
                        __try { hash = *(int*)entry; } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                        if (hash < 0) continue;

                        void* actor = nullptr;
                        __try { actor = *(void**)(entry + 16); } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                        if (!actor || actor == localMech) continue;

                        void* actorTeam = nullptr;
                        __try { actorTeam = *(void**)((char*)actor + 0xA0); } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                        if (actorTeam == localTeam) continue;

                        void* hc = nullptr;
                        float hp = 0.f;
                        __try { hc = *(void**)((char*)actor + 0x78); } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                        if (!hc) continue;
                        __try { hp = *(float*)((char*)hc + 0x18); } __except(EXCEPTION_EXECUTE_HANDLER){ continue; }
                        if (hp <= 0.f) continue;

                        if (g_IgnoreAbsorber && g_HasEffect && g_HasEffect(actor, 76, nullptr)) continue;
                        if (g_IgnorePhaseShift && g_HasEffect && g_HasEffect(actor, 42, nullptr)) continue;
                        if (g_IgnoreCounterShield && g_HasEffect && g_HasEffect(actor, 70, nullptr)) continue;

                        if (reflectorCount > 0)
                        {
                            void* xfm = nullptr;
                            Vector3 pos{};
                            bool havePos = false;
                            __try { xfm = g_GetTransform(actor, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){ xfm = nullptr; }
                            if (xfm)
                            {
                                __try { g_GetPosition(&pos, xfm, nullptr); havePos = true; }
                                __except(EXCEPTION_EXECUTE_HANDLER) { havePos = false; }
                            }
                            if (!havePos) continue;

                            float minDistSq = 1e30f;
                            for (int ri = 0; ri < reflectorCount; ri++)
                            {
                                float dx = pos.x - reflectorPos[ri].x;
                                float dy = pos.y - reflectorPos[ri].y;
                                float dz = pos.z - reflectorPos[ri].z;
                                float d2 = dx*dx + dy*dy + dz*dz;
                                if (d2 < minDistSq) minDistSq = d2;
                            }

                            if (bestEnemy == nullptr || minDistSq > bestMinDistSq)
                            {
                                bestEnemy = actor;
                                bestMinDistSq = minDistSq;
                            }
                        }
                        else
                        {
                            if (hp < bestHp) { bestHp = hp; bestEnemy = actor; }
                        }
                    }

                    if (bestEnemy)
                        s_gotoEnemyTarget = bestEnemy;
                }
                else
                {
                    bestEnemy = currentTarget;
                }

                // Rigidbody used to zero velocity immediately after a teleport
                // (suppresses the gravity impulse — not needed every frame).
                void* localRb = nullptr;
                __try { localRb = *(void**)((char*)localMech + 0xC8); } __except(EXCEPTION_EXECUTE_HANDLER){}

                if (bestEnemy)
                {
                    if (g_HasEffect)
                    {
                        bool targetIgnored = false;
                        if (g_IgnoreAbsorber && g_HasEffect(bestEnemy, 76, nullptr)) targetIgnored = true;
                        if (g_IgnorePhaseShift && g_HasEffect(bestEnemy, 42, nullptr)) targetIgnored = true;
                        if (g_IgnoreCounterShield && g_HasEffect(bestEnemy, 70, nullptr)) targetIgnored = true;
                        if (targetIgnored)
                        {
                            s_gotoEnemyTarget = nullptr;
                            bestEnemy = nullptr;
                        }
                    }
                }

                if (bestEnemy)
                {
                    // Current health (hc+0x18) is re-evaluated every frame so we always
                    // follow the weakest living enemy, rechecking between each teleport.
                    void* enemyXfm = nullptr;
                    __try { enemyXfm = g_GetTransform(bestEnemy, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                    if (enemyXfm)
                    {
                        Vector3 targetPos{};
                        __try { g_GetPosition(&targetPos, enemyXfm, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                        targetPos.x += g_GotoEnemyOffsetX;
                        targetPos.y += g_GotoEnemyOffsetY;
                        targetPos.z += g_GotoEnemyOffsetZ;

                        // Always update the ghost target so the NetworkSerialize
                        // hook has a fresh position regardless of which mode is on.
                        s_ghostGotoTargetPos = targetPos;
                        s_ghostGotoHasTarget = true;

                        if (g_AutoGotoEnemy)
                        {
                            void* localXfm = nullptr;
                            __try { localXfm = g_GetTransform(localMech, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                            if (localXfm)
                            {
                                __try { g_SetPosition(localXfm, &targetPos, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                                if (localRb && g_SetVelocity)
                                {
                                    Vector3 zeroVel{};
                                    __try { g_SetVelocity(localRb, &zeroVel, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                                }
                            }
                        }
                    }
                }
                else
                {
                    // No living enemies — clear ghost target.
                    s_ghostGotoHasTarget = false;

                    if (g_AutoGotoEnemy)
                    {
                        // Park at staging position (1000, 1000, 1000).
                        void* localXfm = nullptr;
                        __try { localXfm = g_GetTransform(localMech, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                        if (localXfm)
                        {
                            Vector3 stagingPos = { 1000.f, 1000.f, 1000.f };
                            __try { g_SetPosition(localXfm, &stagingPos, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                            if (localRb && g_SetVelocity)
                            {
                                Vector3 zeroVel{};
                                __try { g_SetVelocity(localRb, &zeroVel, nullptr); } __except(EXCEPTION_EXECUTE_HANDLER){}
                            }
                        }
                    }
                }
            }
        }
    }
    // -- Hide Player Name / Player Scale / Camera FOV + Zoom ------------------------------
    if (anyPlayer)
    {
        // -- Hide Player Name: static Player.name (lobby/hangar top bar + reactive UI events)
        // Also writes to FullPlayer.ServerPlayerData.name (in-battle top-left HUD)
        // Runs unconditionally � does NOT require noclip API init
        if (g_HidePlayerName)
        {
            // typedef for static Player.get_ServerPlayerData() ? FullPlayer*
            typedef void* (__fastcall* fn_GetServerPlayerData_t)(void* method);

            static fn_SetPlayerName_t       s_fnSetName = nullptr;
            static fn_Il2cppStrNew_t        s_fnStrNew  = nullptr;
            static fn_GetServerPlayerData_t s_fnGetSPD  = nullptr;
            if (!s_fnSetName) s_fnSetName = (fn_SetPlayerName_t)      (GetBase() + 0xEF2790);
            if (!s_fnStrNew)  s_fnStrNew  = (fn_Il2cppStrNew_t)       GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");
            if (!s_fnGetSPD)  s_fnGetSPD  = (fn_GetServerPlayerData_t)(GetBase() + 0xEF18F0);
            if (s_fnSetName && s_fnStrNew)
            {
                static void* s_fakeStr = nullptr;
                if (!s_fakeStr) s_fakeStr = MakePinnedString("<a>TitanCore</a>");
                void* fakeStr = s_fakeStr;
                if (fakeStr)
                {
                    // 1. Player._name + fire PlayerNameChangedEvent (lobby top bar & menu observers)
                    __try { s_fnSetName(fakeStr, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                    // 2. FullPlayer.name at +0x28 (battle top-left HUD reads ServerPlayerData directly)
                    if (s_fnGetSPD)
                    {
                        void* fullPlayer = nullptr;
                        __try { fullPlayer = s_fnGetSPD(nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                        if (fullPlayer)
                            __try { *reinterpret_cast<void**>((char*)fullPlayer + 0x28) = fakeStr; } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    }
                }
            }
        }

        if (EnsureNoclipAPI())
        {
            // -- One-time CinemachineBrain.LateUpdate hook install (camera zoom) --------
            // This hook fires after Cinemachine commits its camera position each frame,
            // which is the correct time to override it — before Unity's render pass.
            if (!s_brainLUHooked)
            {
                void* brainLUTarget = reinterpret_cast<void*>(GetBase() + 0x1487730);
                if (MH_CreateHook(brainLUTarget,
                                  reinterpret_cast<void*>(&hkBrainLateUpdate),
                                  reinterpret_cast<void**>(&o_BrainLateUpdate)) == MH_OK)
                {
                    MH_EnableHook(brainLUTarget);
                    s_brainLUHooked = true;
                }
            }

            // -- Hide Player Name: Actor.ActorName (in-battle nameplate & battle HUD)
            // set_ActorName RVA 0x73B500 � instance method on Actor base class
            if (g_HidePlayerName)
            {
                static fn_SetActorName_t s_fnSetActorName = nullptr;
                static fn_Il2cppStrNew_t s_fnStrNewActor  = nullptr;
                if (!s_fnSetActorName) s_fnSetActorName = (fn_SetActorName_t)(GetBase() + 0x748E90);
                if (!s_fnStrNewActor)  s_fnStrNewActor   = (fn_Il2cppStrNew_t)GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");
                void* mech = GetActiveMech();
                if (mech && s_fnSetActorName && s_fnStrNewActor)
                {
                    static void* s_fakeStrActor = nullptr;
                    if (!s_fakeStrActor) s_fakeStrActor = MakePinnedString("<a>TitanCore</a>");
                    if (s_fakeStrActor) __try { s_fnSetActorName(mech, s_fakeStrActor, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                }

                // 4. LiteNetPlayer.<Name>k__BackingField (+0x20) � in-battle top-left HUD
                // Chain: MatchContext.NetworkService ? get_Engine() ? _players(+0x38)
                //        ? get_LocalId() ? Find(id) ? write +0x20
                if (g_NetworkSvcField && g_FieldStaticGet)
                {
                    typedef void* (__fastcall* fn_GetEngine_t) (void* instance, void* method);
                    typedef int   (__fastcall* fn_GetLocalId_t)(void* instance, void* method);
                    typedef void* (__fastcall* fn_FindPlayer_t)(void* instance, int id, void* method);

                    static fn_GetEngine_t  s_fnGetEng  = nullptr;
                    static fn_GetLocalId_t s_fnGetLId  = nullptr;
                    static fn_FindPlayer_t s_fnFind    = nullptr;
                    static fn_Il2cppStrNew_t s_fnStrNewLN = nullptr;
                    if (!s_fnGetEng)  s_fnGetEng  = (fn_GetEngine_t) (GetBase() + 0x721A90);
                    if (!s_fnGetLId)  s_fnGetLId  = (fn_GetLocalId_t)(GetBase() + 0x87B670);
                    if (!s_fnFind)    s_fnFind    = (fn_FindPlayer_t) (GetBase() + 0x852C190);
                    if (!s_fnStrNewLN) s_fnStrNewLN = (fn_Il2cppStrNew_t)GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");

                    void* netSvc = nullptr;
                    __try { g_FieldStaticGet(g_NetworkSvcField, &netSvc); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    if (netSvc && s_fnGetEng)
                    {
                        void* engine = nullptr;
                        __try { engine = s_fnGetEng(netSvc, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                        if (engine)
                        {
                            void* players = nullptr;
                            __try { players = *reinterpret_cast<void**>((char*)engine + 0x38); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                            if (players && s_fnGetLId && s_fnFind && s_fnStrNewLN)
                            {
                                int localId = 0;
                                __try { localId = s_fnGetLId(players, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                void* localPlayer = nullptr;
                                __try { localPlayer = s_fnFind(players, localId, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                if (localPlayer)
                                {
                                    static void* s_fakeStrLN = nullptr;
                                    if (!s_fakeStrLN) s_fakeStrLN = MakePinnedString("<a>TitanCore</a>");
                                    if (s_fakeStrLN)
                                        __try { *reinterpret_cast<void**>((char*)localPlayer + 0x20) = s_fakeStrLN; } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                }
                            }
                        }
                    }
                }

                // 5. Squad/social instanced Player.<Name>k__BackingField (+0x18)
                // Covers: squad panel, custom game team menu
                // Chain: PlayerContext._squadManager ? SquadManager._squad(+0x20)
                //        ? MySquad.<SquadSlots>k__BackingField(+0x18) ? List<SquadSlot>
                //        ? slot.PlayerProfile(+0x18) ? Player(+0x10)
                //        ? match Player.Id(+0x10) vs PlayerLoginService._playerId(+0x18)
                //        ? write Player.<Name>(+0x18)
                if (g_LoginSvcField && g_SquadMgrField && g_FieldStaticGet)
                {
                    static fn_Il2cppStrNew_t s_fnStrNewSq = nullptr;
                    if (!s_fnStrNewSq) s_fnStrNewSq = (fn_Il2cppStrNew_t)GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");

                    void* loginSvc = nullptr;
                    __try { g_FieldStaticGet(g_LoginSvcField, &loginSvc); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    void* squadMgr = nullptr;
                    __try { g_FieldStaticGet(g_SquadMgrField, &squadMgr); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                    if (loginSvc && squadMgr && s_fnStrNewSq)
                    {
                        // Local player ID: PlayerLoginService._playerId at +0x18
                        void* localIdStr = nullptr;
                        __try { localIdStr = *reinterpret_cast<void**>((char*)loginSvc + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                        // SquadManager._squad (ISquad*) at +0x20
                        void* squad = nullptr;
                        __try { squad = *reinterpret_cast<void**>((char*)squadMgr + 0x20); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                        if (localIdStr && squad)
                        {
                            // MySquad.<SquadSlots>k__BackingField at +0x18 ? List<SquadSlot>*
                            void* slotsList = nullptr;
                            __try { slotsList = *reinterpret_cast<void**>((char*)squad + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                            if (slotsList)
                            {
                                // List<T>: _items array at +0x10, _size int at +0x18
                                void* slotsArr = nullptr; int slotCount = 0;
                                __try {
                                    slotsArr  = *reinterpret_cast<void**>((char*)slotsList + 0x10);
                                    slotCount = *reinterpret_cast<int*>  ((char*)slotsList + 0x18);
                                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                                int32_t localIdLen = 0;
                                __try { localIdLen = *reinterpret_cast<int32_t*>((char*)localIdStr + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                                for (int si = 0; si < slotCount && si < 8 && slotsArr; si++)
                                {
                                    void* slot = nullptr;
                                    __try { slot = *reinterpret_cast<void**>((char*)slotsArr + 0x20 + si * 8); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    if (!slot) continue;
                                    void* playerProfile = nullptr;
                                    __try { playerProfile = *reinterpret_cast<void**>((char*)slot + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    if (!playerProfile) continue;
                                    void* playerObj = nullptr;
                                    __try { playerObj = *reinterpret_cast<void**>((char*)playerProfile + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    if (!playerObj) continue;
                                    void* playerIdStr = nullptr;
                                    __try { playerIdStr = *reinterpret_cast<void**>((char*)playerObj + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    if (!playerIdStr) continue;

                                    bool idMatch = false;
                                    __try {
                                        int32_t pLen = *reinterpret_cast<int32_t*>((char*)playerIdStr + 0x10);
                                        if (pLen == localIdLen && localIdLen > 0)
                                            idMatch = (memcmp((char*)localIdStr + 0x14, (char*)playerIdStr + 0x14, (size_t)localIdLen * 2) == 0);
                                    } __except (EXCEPTION_EXECUTE_HANDLER) {}

                                    if (idMatch)
                                    {
                                        static void* s_fakeStrSq = nullptr;
                                        if (!s_fakeStrSq) s_fakeStrSq = MakePinnedString("<a>TitanCore</a>");
                                        if (s_fakeStrSq)
                                            __try { *reinterpret_cast<void**>((char*)playerObj + 0x18) = s_fakeStrSq; } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    }
                                }
                            }
                        }
                    }
                }

                // 6. DuelSlot Profile.Player.<Name>k__BackingField (+0x18)
                // Covers: custom game team menu
                // Chain: PlayerContext.DuelManager -> DuelManager.<Duel>(+0x18)
                //        -> DuelInfo.<Team1>(+0x20) + <Team2>(+0x28)
                //        -> DuelTeam.<Slots>(+0x20) -> List<DuelSlot>
                //        -> match DuelSlot.<PlayerId>(+0x10) vs localId
                //        -> DuelSlot.<Profile>(+0x38) -> Player(+0x10) -> Name(+0x18)
                if (g_DuelMgrField && g_LoginSvcField && g_FieldStaticGet)
                {
                    static fn_Il2cppStrNew_t s_fnStrNewDu = nullptr;
                    if (!s_fnStrNewDu) s_fnStrNewDu = (fn_Il2cppStrNew_t)GetProcAddress((HMODULE)GetBase(), "il2cpp_string_new");

                    void* loginSvcD = nullptr;
                    __try { g_FieldStaticGet(g_LoginSvcField, &loginSvcD); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    void* duelMgr = nullptr;
                    __try { g_FieldStaticGet(g_DuelMgrField, &duelMgr); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                    if (loginSvcD && duelMgr && s_fnStrNewDu)
                    {
                        void* localIdStrD = nullptr;
                        __try { localIdStrD = *reinterpret_cast<void**>((char*)loginSvcD + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                        int32_t localIdLenD = 0;
                        if (localIdStrD)
                            __try { localIdLenD = *reinterpret_cast<int32_t*>((char*)localIdStrD + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                        void* duelInfo = nullptr;
                        __try { duelInfo = *reinterpret_cast<void**>((char*)duelMgr + 0x18); } __except (EXCEPTION_EXECUTE_HANDLER) {}

                        if (localIdStrD && duelInfo)
                        {
                            for (int t = 0; t < 2; t++)
                            {
                                void* team = nullptr;
                                __try { team = *reinterpret_cast<void**>((char*)duelInfo + 0x20 + t * 8); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                if (!team) continue;

                                void* slotsListD = nullptr;
                                __try { slotsListD = *reinterpret_cast<void**>((char*)team + 0x20); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                if (!slotsListD) continue;

                                void* slotsArrD = nullptr; int slotCountD = 0;
                                __try {
                                    slotsArrD  = *reinterpret_cast<void**>((char*)slotsListD + 0x10);
                                    slotCountD = *reinterpret_cast<int*>  ((char*)slotsListD + 0x18);
                                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                                for (int si = 0; si < slotCountD && si < 12 && slotsArrD; si++)
                                {
                                    void* slot = nullptr;
                                    __try { slot = *reinterpret_cast<void**>((char*)slotsArrD + 0x20 + si * 8); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    if (!slot) continue;

                                    void* slotIdStr = nullptr;
                                    __try { slotIdStr = *reinterpret_cast<void**>((char*)slot + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    if (!slotIdStr) continue;

                                    bool idMatchD = false;
                                    __try {
                                        int32_t pLen = *reinterpret_cast<int32_t*>((char*)slotIdStr + 0x10);
                                        if (pLen == localIdLenD && localIdLenD > 0)
                                            idMatchD = (memcmp((char*)localIdStrD + 0x14, (char*)slotIdStr + 0x14, (size_t)pLen * 2) == 0);
                                    } __except (EXCEPTION_EXECUTE_HANDLER) {}

                                    if (idMatchD)
                                    {
                                        void* profile = nullptr;
                                        __try { profile = *reinterpret_cast<void**>((char*)slot + 0x38); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                        if (!profile) continue;
                                        void* playerObjD = nullptr;
                                        __try { playerObjD = *reinterpret_cast<void**>((char*)profile + 0x10); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                        if (!playerObjD) continue;
                                        static void* s_fakeStrDu = nullptr;
                                        if (!s_fakeStrDu) s_fakeStrDu = MakePinnedString("<a>TitanCore</a>");
                                        if (s_fakeStrDu)
                                            __try { *reinterpret_cast<void**>((char*)playerObjD + 0x18) = s_fakeStrDu; } __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // -- Player Scale ----------------------------------------------
            if (g_PlayerScaleSlider != 0.0f && g_GetTransform && g_GetLocalScale && g_SetLocalScale)
            {
                static void*   s_scaleMech = nullptr;
                static Vector3 s_origScale = {1.f, 1.f, 1.f};
                void* mech = GetActiveMech();
                if (mech)
                {
                    void* tf = nullptr;
                    __try { tf = g_GetTransform(mech, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    if (tf)
                    {
                        if (mech != s_scaleMech)
                        {
                            s_scaleMech = mech;
                            __try { g_GetLocalScale(&s_origScale, tf, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                        }
                        float mult = (g_PlayerScaleSlider >= 0.f)
                            ? (1.f + g_PlayerScaleSlider * 0.09f)    // 0->1x  100->10x
                            : (1.f + g_PlayerScaleSlider * 0.009f);  // 0->1x  -100->0.1x
                        Vector3 scaled = { s_origScale.x * mult, s_origScale.y * mult, s_origScale.z * mult };
                        __try { g_SetLocalScale(tf, &scaled, nullptr); } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    }
                }
                else
                {
                    s_scaleMech = nullptr; // reset on death so original scale re-sampled on respawn
                }
            }

            // -- Camera FOV + Zoom via CinemachineFreeLook -------------------
            // Cinemachine overrides Camera.fieldOfView every frame, so we must
            // write directly to the active FreeLook camera's LensSettings and
            // orbit radii instead.
            //
            // Chain:
            //   CinemachineCore.get_Instance()  RVA 0x14C3A30  ? core
            //   core.GetActiveBrain(0)           RVA 0x870BE10  ? brain
            //   brain.get_ActiveVirtualCamera()  RVA 0x148A480  ? FreeLook*
            //
            // FreeLook layout:
            //   +0xBC : LensSettings (struct, inline) � FieldOfView at +0x0
            //   +0x240: m_Orbits (IL2CPP array*)
            //           array+0x20+i*8+4 ? Orbit[i].m_Radius
            if (g_CameraFOV > 0.0f || s_fovWasActive)
            {
                typedef void* (__fastcall* fn_GetCore_t) (void*);
                typedef void* (__fastcall* fn_GetBrain_t)(void*, int, void*);
                typedef void* (__fastcall* fn_GetVCam_t) (void*, void*);

                void* core = nullptr;
                __try {
                    auto fnGetCore = (fn_GetCore_t)(GetBase() + 0x14C3A30);
                    core = fnGetCore(nullptr);
                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                if (core)
                {
                    void* brain = nullptr;
                    __try {
                            auto fnGetBrain = (fn_GetBrain_t)(GetBase() + 0x870BE10);
                    } __except (EXCEPTION_EXECUTE_HANDLER) {}

                    if (brain)
                    {
                        void* vcam = nullptr;
                        __try {
                            auto fnGetVCam = (fn_GetVCam_t)(GetBase() + 0x148A480);
                            vcam = fnGetVCam(brain, nullptr);
                        } __except (EXCEPTION_EXECUTE_HANDLER) {}

                        if (vcam)
                        {
                            static float s_origFOV      = 0.0f;
                            static bool  s_origFOVSaved = false;
                            if (g_CameraFOV > 0.0f)
                            {
                                // Save the original FOV once before we first overwrite it.
                                if (!s_origFOVSaved)
                                {
                                    __try { s_origFOV = *(float*)((char*)vcam + 0xBC); }
                                    __except (EXCEPTION_EXECUTE_HANDLER) {}
                                    s_origFOVSaved = true;
                                }
                                // g_CameraFOV is horizontal degrees (CoD convention).
                                // Convert to vertical FOV (16:9) then write to vcam+0xBC.
                                const float vFov = 2.0f * atanf(tanf(g_CameraFOV * 3.14159265f / 360.0f) / (16.0f / 9.0f)) * (180.0f / 3.14159265f);
                                __try { *(float*)((char*)vcam + 0xBC) = vFov; }
                                __except (EXCEPTION_EXECUTE_HANDLER) {}
                                s_fovWasActive = true;
                            }
                            else // FOV slider moved back to 0: restore original value
                            {
                                __try { *(float*)((char*)vcam + 0xBC) = s_origFOV; }
                                __except (EXCEPTION_EXECUTE_HANDLER) {}
                                s_fovWasActive  = false;
                                s_origFOVSaved  = false; // re-capture on next enable
                            }
                        }
                    }
                }
            }
        }
    }
}
