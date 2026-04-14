# War Robots Mod — Offsets & RVA Reference

All values are relative to `GameAssembly.dll` base address (`GetBase()`).
When the game updates, find these symbols in the new dump (WRDump.txt) to get updated values.
Dump search tip: `findstr /n /c:"MethodName" "WRDump.txt"`

---

## Function RVAs  (`GetBase() + RVA`)

### Bot / AI shooting (nuke-bot feature)
| RVA | Symbol |
|---|---|
| `0x130B380` | `BotShooter.Shoot(int count, Tags)` |
| `0x130B2D0` | `BotShooter.Shoot(int slot, int count, Tags)` |
| `0x130B100` | `BotShooter.Shoot(IEquipmentSlot, int, Tags)` — static helper |
| `0x1316C40` | `ShootExecutor.Tick(Context, ActionContext)` |
| `0xB01D50`  | `BotMechController.ShootWeaponInRange(Target)` |
| `0x13178D0` | `TowerExecutor.Tick(Context, ActionContext)` |

### Ability lock bypasses (patch-to-return-false)
| RVA | Symbol |
|---|---|
| `0xE9BD80` | `MechStats.get_Ability1LockedByPilot()` |
| `0xE9BDC0` | `MechStats.get_Ability2LockedByPilot()` |
| `0xE9BDE0` | `MechStats.get_AbilityLockedByStatusEffect()` |
| `0xE9BE60` | `MechStats.get_ModuleLockedByStatusEffect()` |

### Target lock / effect system
| RVA | Symbol |
|---|---|
| `0xA90080` | `ActorExtensions.HasEffect(IActor, EffectKind)` |
| `0xE97C10` | `MechStats.get_TargetLockSpeed()` |

### AOE radius return patches
| RVA | Symbol |
|---|---|
| `0xE97AF0` | `EquipmentStats.get_AoeRadiusMultiplier()` |
| `0xE97AD0` | `EquipmentStats.get_AoeRadiusAdditional()` |

### Transform / Physics (Unity internals)
| RVA | Symbol |
|---|---|
| `0x99CA30`   | `Actor.get_Transform()` |
| `0x4D2B870`  | `Component.get_transform()` (for Camera) |
| `0x4D3FB70`  | `Transform.get_position()` — Vector3 hidden-return |
| `0x4D403C0`  | `Transform.set_position()` — Vector3 by pointer |
| `0x4D3F610`  | `Transform.get_forward()` — Vector3 hidden-return |
| `0x4D3FBC0`  | `Transform.get_right()` — Vector3 hidden-return |
| `0x4D3F930`  | `Transform.get_localScale_Injected()` |
| `0x4D40210`  | `Transform.set_localScale_Injected()` |
| `0x4DBD310`  | `Rigidbody.set_velocity()` — Vector3 by pointer |
| `0x4DBCA00`  | `Rigidbody.get_velocity_Injected()` — Vector3 out param |
| `0x4CF3A70`  | `Camera.get_main()` — static, returns Camera* in RAX |
| `0x1037C60`  | `AnimationController.get_RotationSpeed()` |
| `0xF70460`   | `Turret.get_horizRotationSpeed()` |
| `0x145E000`  | `CinemachineCore.get_Instance()` |
| `0x86E5A80`  | `CinemachineCore.GetActiveBrain(int)` |
| `0x1421CD0`  | `CinemachineBrain.LateUpdate()` |
| `0x1424A20`  | `Brain.get_ActiveVirtualCamera()` — FreeLook* |

### Ability system
| RVA | Symbol |
|---|---|
| `0x12340A0` | `AbilityHudController.get_Instance()` — static singleton |
| `0xF36B20`  | `NetworkAbilityCustomizable.GotoNextState()` |

### Weapon system
| RVA | Symbol |
|---|---|
| `0x90CE70`  | `Weapon.IShootingWeapon.ScheduleShots(int count)` |
| `0x90D690`  | `Weapon.IShootingWeapon.set_ShotInterval(float)` |
| `0x90D130`  | `Weapon.IShootingWeapon.set_AmmoConsumptionPerShot(float)` |
| `0x1257370` | `CreateHitEventData(...)` — damage packet hook |

### Quest / Reward API
| RVA | Symbol / Use |
|---|---|
| `0x82803A0` | `GetQuestService()` / quest system entry |
| `0x10F0D40` | `TryFinishAllQuests(questService, questTabs)` |
| `0x827F620` | `GetPveModel()` |
| `0x7E1340`  | `PveModel.GetState()` |
| `0x1196E80` | `PveUtils.GetLayout(playerState)` |
| `0x5029720` | `PvE20RewardAllAsync(request, ...)` |

### Phoenix kill streak / multi-kill text
| RVA | Symbol / Use |
|---|---|
| `0xD37860` | `MultiKillView.SetText(...)` / phoenix kill streak text hook |
| `0x100E8B0` | `MultiKillService.GetLocKey(...)` |

### Network / State hooks
| RVA | Symbol / Use |
|---|---|
| `0x76F4000` | `NetMechDynamics.GetState()` — Net dynamics state (Float3Position: X@+0x0, Y@+0x4, Z@+0x8) |
| `0x87C490`  | `IsGrounded()` — client-side IsGrounded hook (force grounded) |

### Player name hiding
| RVA | Symbol |
|---|---|
| `0xE87080`  | `Player.set_name(string)` — static Player class (lobby bar) |
| `0xE86690`  | `Player.get_name()` — static Player class |
| `0xE861E0`  | `get_ServerPlayerData()` — returns FullPlayer* |
| `0x73B500`  | `Actor.set_ActorName(string)` — kill feed |
| `0x721910`  | `NetworkService.get_Engine()` |
| `0x875D90`  | `PlayerCollection.get_LocalId()` |
| `0x84A6BD0` | `PlayerCollection.Find(int id)` |

### Misc Unity
| RVA | Symbol |
|---|---|
| `0x4CF0DD0` | `Application.set_targetFrameRate(int)` — static |

---

## Static Fields  (resolved at runtime via il2cpp reflection)

These are **not hardcoded offsets** — found by name via `il2cpp_class_get_field_from_name`.
If the field name changes, update the string in `InitNoclipAPI()`.

| Class | Namespace | Field name | Used for |
|---|---|---|---|
| `MatchContext` | `Game.Core` | `MechManagement` | mech/weapon/ability chains |
| `MatchContext` | `Game.Core` | `NetworkService` | LiteNetPlayer name |
| `PlayerContext` | `Game.Core` | `PlayerLoginService` | local player ID |
| `PlayerContext` | `Game.Core` | `_squadManager` | squad panel names |
| `PlayerContext` | `Game.Core` | `DuelManager` | custom game team names |

---

## Struct Field Offsets

### GetActiveMech pointer chain
```
MatchContext.MechManagement (static Il2CppField*)
  → MechManagementService._playerMechState     +0x30
    → PlayerMechState.IsMechActive (bool)       +0x40
    → PlayerMechState.ActiveMech (IActor*)      +0x30
      → Mech.<rigidbody>k__BackingField         +0xC8
```

### Weapon / shooting chain
```
Mech
  → _shootingSlotCollectionManager              +0x258
    → ShootingSlotCollectionManager._slots[]    +0x18  (IL2CPP array: count @+0x18, items @+0x20)
      [i] ShootingSlotManager
        → _weapons  List<IShootingWeapon>        +0x10
          → List._items[]                        +0x10
            [j] Weapon
              → _ammoCapacity  (float)           +0x14C
              → _ammoValue     (float)           +0x154  (current ammo)
              → _shotInterval  (float)           +0x158
              → _scheduledShots (int)            +0x178
              → _isReloading   (bool)            +0x17C
              → _absTime       (float)           +0x188
```

### Ability system
```
AbilityHudController (singleton via get_Instance() RVA 0x12340A0)
  → _abilities[]  (IL2CPP array)                +0x40
  → _modules[]    (IL2CPP array)                +0x48
    [i] NetworkAbilityCustomizable (NAC)
      → _states[] (IL2CPP array)                +0x60
        [1]     AbilityStateWithDuration
          → _duration       (float)             +0x68
          → _activateTime   (float)             +0x48
          → _endDurationTime (float)            +0x70
        [last]  AbilityStateWithCooldown
          → _cooldown       (float)             +0x70
```

### MechRegistry (ESP / auto-goto-enemy)
```
MatchContext.MechManagement (static)
  → MechManagementService._mechRegistry         +0x38
    → MechRegistry._mechs  Dictionary<ViewId,IActor>  +0x10
      → Entry[] array                           +0x18
      → int count                               +0x20
      Entry layout (24 bytes):
        +0x00  int   hash      (< 0 = deleted)
        +0x04  int   next
        +0x08  uint  ViewId key
        +0x0C  int   padding
        +0x10  void* IActor* value
  BaseCombatActor
    → <Team>k__BackingField                     +0xA0
    → _healthController                         +0x78
      → HealthController._health (float)        +0x18
```

### Player name — lobby / menus  (static Player class)
```
Player.set_name(string)  RVA 0xE87080   ← call each frame
  writes static Player._name
FullPlayer (from get_ServerPlayerData() RVA 0xE861E0)
  → .name  (string)                            +0x28
```

### Player name — in-battle HUD (LiteNetPlayer)
```
MatchContext.NetworkService (static)
  → NetworkService.get_Engine()  RVA 0x721910
    → engine._players (PlayerCollection)        +0x38
      → PlayerCollection.get_LocalId()  RVA 0x875D90
      → PlayerCollection.Find(id)        RVA 0x84A6BD0 → LiteNetPlayer*
        → LiteNetPlayer.<Name>k__BackingField   +0x20  ← WRITE
```

### Player name — kill feed (Actor)
```
Actor.set_ActorName(string)  RVA 0x73B500   ← call each frame
```

### Player name — squad panel  (instanced Player)
```
PlayerContext._squadManager (static)
  → SquadManager._squad (ISquad*)              +0x20
    → MySquad.<SquadSlots>k__BackingField       +0x18  (List<SquadSlot>)
      List._items[]                             +0x10
      List._size  (int)                         +0x18
      [i] SquadSlot
        → <PlayerProfile>k__BackingField        +0x18
          → PlayerProfile.<Player>              +0x10  (instanced Player, TypeDef 17284)
            → <Id>k__BackingField   (string)    +0x10  ← compare vs local ID
            → <Name>k__BackingField (string)  +0x18  ← WRITE

PlayerContext.PlayerLoginService (static)
  → PlayerLoginService._playerId (string)       +0x18  ← local player's server ID
```

### Player name — custom game team menu  (DuelSlot)
```
PlayerContext.DuelManager (static)
  → DuelManager.<Duel>k__BackingField           +0x18  (DuelInfo*)
    → DuelInfo.<Team1>k__BackingField           +0x20  (DuelTeam*)
    → DuelInfo.<Team2>k__BackingField           +0x28  (DuelTeam*)
    [t=0,1] DuelTeam
      → <Slots>k__BackingField                  +0x20  (List<DuelSlot>)
        List._items[]                           +0x10
        List._size  (int)                         +0x18
        [i] DuelSlot
          → <PlayerId>k__BackingField (string)  +0x10  ← compare vs local ID
          → <Profile>k__BackingField            +0x38  (PlayerProfile*)
            → PlayerProfile.<Player>            +0x10  (instanced Player)
              → <Name>k__BackingField (string)  +0x18  ← WRITE
```

---

## IL2CPP String layout
```
+0x00  void*  klass pointer
+0x08  void*  monitor
+0x10  int32  length  (char count, NOT bytes)
+0x14  wchar  chars[] (UTF-16, 2 bytes each)
```

---

## How to update after a game patch

1. Open the new dump in `WRDump.txt`.
## What to update after a game patch — quick checklist

When the game updates, these are the things you will most commonly need to update in the codebase:

- **Function RVAs (GetBase() + RVA):** most live as inline hex constants in `src/patches.cpp` (e.g. `GetBase() + 0x82803A0`). Search the repo for `GetBase() + 0x` to find all usages and update the hex value to the new RVA from the dump.
- **IL2CPP static fields (resolved at runtime):** these are looked up by name via the il2cpp API (see `ResolveIl2CppClass` / `il2cpp_class_get_field_from_name` in `src/patches.cpp`). Only change these if the field *name* changed in the dump.
- **Struct field offsets / layouts:** these are the +0xNN offsets inside structs (e.g. `Mech._rigidbody +0xC8`). Update these when the class field layout in the dump shows different offsets.
- **Search for hardcoded hex constants:** besides `GetBase()+RVA` there are other literal offsets used for pointer arithmetic — grep for `0x[0-9A-Fa-f]{2,8}` to locate candidates.

Files to inspect when updating:

- `src/patches.cpp` — primary location for `GetBase() + 0x...` entries and il2cpp lookups.
- `src/patches.h` / public externs — cross-check declarations.
- Any `offsets.h`/`Offsets.h` or `sdk.h` in sibling projects — these may hold re-usable constants.

Useful quick commands

- Find all inline RVA uses (PowerShell):

```powershell
Select-String -Path .\src\**\*.cpp -Pattern 'GetBase\(\)\s*\+\s*0x[0-9A-Fa-f]+' -AllMatches
```

- Find method entries in a dump (Windows `findstr`):

```cmd
findstr /n /c:"GetQuestService" WRDump.txt
```

- Extract RVA for a named method from `WRDump.txt` (PowerShell):

```powershell
$name = 'GetQuestService'
$line = Select-String -Path WRDump.txt -Pattern $name -SimpleMatch -List | Select-Object -First 1
if ($line -and $line.Line -match 'RVA 0x([0-9A-Fa-f]+)') { "{0} -> 0x{1}" -f $name,$matches[1] }
```

Automation tip: you can keep a short list of method names (from this file) and loop over them with the script above to print new RVAs, then paste the hex constants back into `src/patches.cpp`.

Verification checklist after updating

- Rebuild the DLL and load into the game; check for crashes on start.
- Open the mod log (`WarRobotsMod.log`) — `PLog` calls in `src/patches.cpp` can help diagnose missing symbols.
- Temporarily instrument `patches.cpp` to `PLog()` the resolved function pointers (print pointer values) so you can verify the addresses are non-null.

2. For each RVA, search:  `findstr /n /c:"MethodName" WRDump.txt`  — the `// RVA 0x...` comment on that line is the new value.
3. For struct field offsets:  `findstr /n /c:"class StructName" WRDump.txt`  — read the field list; the `// 0x...` comment after each field declaration is the offset.
4. For static fields resolved by name (`InitNoclipAPI`): only update if the field was renamed in the dump.