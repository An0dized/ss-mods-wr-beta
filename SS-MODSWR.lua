if gg.getTargetPackage() == "com.pixonic.wwr" then
else
  gg.alert("This Script is Made for WR please select the process in top corner")
end

repeat
  local w = gg.choice(
    { "Press to continue✅" },
    nil,
    "⚠️  IMPORTANT  ⚠️\n\nTHIS SCRIPT IS STILL IN BETA DONT EXPECT EVERYTHING TO FUNCTION CORRECTLY. ALSO MAKE SURE TO ENABLE ALL MODS IN THE HANGER OR ELSE THEY WON'T WORK!\n\n— SS-MODS —"
  )
  if w == 1 then break end
until false

-- ══════════════════════════════════════
--  Feature State Variables
-- ══════════════════════════════════════
local speedHackEnabled = false
local gravityEnabled = false
local noclipEnabled = false
local skill1Enabled = false
local durationSkill1Enabled = false
local skill2Enabled = false
local durationSkill2Enabled = false
local infiniteModuleEnabled = false
local multipleJumpEnabled = false
local noJumpEnabled = false
local antiEMPEnabled = false
local antiSkillEnabled = false
local bigRadiusEnabled = false
local instantBoxEnabled = false
local botsDontShootEnabled = false
local shortDistanceEnabled = false
local instantAdSkipEnabled = false
local instantUpgradeEnabled = false
local godModeEnabled = false
local rapidFireEnabled = false
local weaponRangeEnabled = false
local noReloadEnabled = false
local noRecoilEnabled = false
local autoFireEnabled = false
local titanChargeEnabled = false
local antiLockdownEnabled = false
local antiBlindEnabled = false
local damageMultEnabled = false
local infiniteAmmoEnabled = false
local infiniteHpEnabled = false
local antiDeathEnabled = false
local targetLockEnabled = false
local fovEnabled = false
local accuracyEnabled = false
local projectileSpeedEnabled = false
local antiStealthEnabled = false
local teleportBeaconEnabled = false
local undergroundEnabled = false
local matchTimeEnabled = false
local infiniteShieldEnabled = false
local turretSpeedEnabled = false
local chargeRateEnabled = false
local energyRefreshEnabled = false
local superJumpEnabled = false
local maxHpEnabled = false
local autoAbility1Enabled = false
local autoAbility2Enabled = false
local autoAbilityBothEnabled = false


local dropOpen = {
  movement   = false,
  abilities  = false,
  weapons    = false,
  defense    = false,
  targeting  = false,
  teleport   = false,
  match      = false,
  bots       = false,
  misc       = false,
}

HOME = 1
function HOME()
  local items = {}

  local function add(display, key)
    items[#items + 1] = { display, key }
  end

  local function tog(enabled) return enabled and "☑ " or "☐ " end
  local function sec(open)    return open   and "▼ " or "▶ " end

  -- ── Section headers ──
  add("[ " .. sec(dropOpen.movement)  .. "⚡ MOVEMENT ]",    "hdr_movement")
  if dropOpen.movement then
    add("  " .. tog(speedHackEnabled)    .. "SpeedHack",       "speedHack")
    add("  " .. tog(gravityEnabled)      .. "Disable Gravity",  "gravity")
    add("  " .. tog(noclipEnabled)       .. "Noclip",           "noclip")
    add("  " .. tog(multipleJumpEnabled) .. "Multiple Jump",    "multiJump")
    add("  " .. tog(noJumpEnabled)       .. "No Jump",          "noJump")
    add("  " .. tog(superJumpEnabled)    .. "Super Jump",       "superJump")
  end

  add("[ " .. sec(dropOpen.abilities) .. "⚔️ ABILITIES ]",   "hdr_abilities")
  if dropOpen.abilities then
    add("  " .. tog(skill1Enabled)         .. "Infinite Robot Skill 1",        "skill1")
    add("  " .. tog(durationSkill1Enabled) .. "No Duration Robot Skill 1",     "durSkill1")
    add("  " .. tog(skill2Enabled)         .. "Infinite Robot Skill 2",        "skill2")
    add("  " .. tog(durationSkill2Enabled) .. "No Duration Robot Skill 2",     "durSkill2")
    add("  " .. tog(infiniteModuleEnabled) .. "Infinite Module",               "infModule")
    add("  " .. tog(titanChargeEnabled)    .. "Instant Titan Charge",          "titanCharge")
    add("  " .. tog(energyRefreshEnabled)  .. "Fast Energy Refresh",           "energyRefresh")
    add("  " .. tog(autoAbility1Enabled)    .. "🔁 Auto Ability 1",       "autoAbility1")
    add("  " .. tog(autoAbility2Enabled)    .. "🔁 Auto Ability 2",       "autoAbility2")
    add("  " .. tog(autoAbilityBothEnabled) .. "🔁 Auto Ability 1 & 2",   "autoAbilityBoth")
  end

  add("[ " .. sec(dropOpen.weapons)   .. "🔫 WEAPONS ]",     "hdr_weapons")
  if dropOpen.weapons then
    add("  " .. tog(rapidFireEnabled)       .. "Rapid Fire",              "rapidFire")
    add("  " .. tog(autoFireEnabled)        .. "Auto Fire (No Delay)",    "autoFire")
    add("  " .. tog(weaponRangeEnabled)     .. "Weapon Range 1100m",      "weaponRange")
    add("  " .. tog(noReloadEnabled)        .. "No Reload",               "noReload")
    add("  " .. tog(noRecoilEnabled)        .. "No Recoil",               "noRecoil")
    add("  " .. tog(infiniteAmmoEnabled)    .. "Infinite Ammo",           "infAmmo")
    add("  " .. tog(damageMultEnabled)      .. "Damage Multiplier",       "damageMult")
    add("  " .. tog(bigRadiusEnabled)       .. "Big Radius Damage",       "bigRadius")
    add("  " .. tog(chargeRateEnabled)      .. "Instant Charge Weapons",  "chargeRate")
    add("  " .. tog(accuracyEnabled)        .. "Perfect Accuracy",        "accuracy")
    add("  " .. tog(projectileSpeedEnabled) .. "Instant Hit Projectile",  "projSpeed")
    add("  " .. tog(turretSpeedEnabled)     .. "Max Turret Rotation",     "turretSpeed")
  end

  add("[ " .. sec(dropOpen.defense)   .. "🛡️ DEFENSE ]",     "hdr_defense")
  if dropOpen.defense then
    add("  " .. tog(godModeEnabled)        .. "God Mode",               "godMode")
    add("  " .. tog(infiniteHpEnabled)     .. "Infinite HP",            "infHp")
    add("  " .. tog(maxHpEnabled)          .. "Max HP Boost",           "maxHp")
    add("  " .. tog(antiDeathEnabled)      .. "Anti Death",             "antiDeath")
    add("  " .. tog(infiniteShieldEnabled) .. "Infinite Energy Shield", "infShield")
    add("  " .. tog(antiEMPEnabled)        .. "Anti EMP Block",         "antiEMP")
    add("  " .. tog(antiSkillEnabled)      .. "Anti Skill Block",       "antiSkill")
    add("  " .. tog(antiLockdownEnabled)   .. "Anti Lockdown",          "antiLockdown")
    add("  " .. tog(antiBlindEnabled)      .. "Anti Blind",             "antiBlind")
  end

  add("[ " .. sec(dropOpen.targeting) .. "🎯 TARGETING ]",   "hdr_targeting")
  if dropOpen.targeting then
    add("  " .. tog(targetLockEnabled)   .. "Instant Target Lock",    "targetLock")
    add("  " .. tog(antiStealthEnabled)  .. "See Through Stealth",    "antiStealth")
    add("  " .. tog(fovEnabled)          .. "Wide FOV",               "fov")
  end

  add("[ " .. sec(dropOpen.teleport)  .. "🌀 TELEPORT ]",    "hdr_teleport")
  if dropOpen.teleport then
    add("  " .. tog(teleportBeaconEnabled) .. "Teleport to Beacon", "teleportBeacon")
    add("  " .. tog(undergroundEnabled)    .. "Underground Mode",    "underground")
  end

  add("[ " .. sec(dropOpen.match)     .. "🕒 MATCH ]",       "hdr_match")
  if dropOpen.match then
    add("  " .. tog(matchTimeEnabled) .. "Fast Match End", "matchTime")
  end

  add("[ " .. sec(dropOpen.bots)      .. "🤖 BOTS / MAP ]",  "hdr_bots")
  if dropOpen.bots then
    add("  " .. tog(botsDontShootEnabled) .. "Disable Bot AI",  "botsNoShoot")
    add("  " .. tog(shortDistanceEnabled) .. "Short Distance",  "shortDist")
  end

  add("[ " .. sec(dropOpen.misc)      .. "🎁 MISC ]",        "hdr_misc")
  if dropOpen.misc then
    add("  " .. tog(instantBoxEnabled)     .. "Instant Box",     "instBox")
    add("  " .. tog(instantAdSkipEnabled)  .. "No Ads (Skip Popup)", "instAd")
    add("  " .. tog(instantUpgradeEnabled) .. "Instant Upgrade", "instUpgrade")
  end

  add("❓ Help",         "help")
  add("🖕 Exit Script",  "exit")

  -- ── Build display list ──
  local display = {}
  local pretick = {}
  for i, v in ipairs(items) do
    display[i] = v[1]
    pretick[i] = false   -- everything starts unchecked by default
  end

  -- ── Pre-tick ONLY the dropdown HEADERS that are currently open.
  --    Mod checkboxes are intentionally left unchecked every time the
  --    menu opens — checking one = "press to toggle", it auto-resets
  --    to unchecked after OK so the menu always looks clean.
  --    The *label text* already shows ☑/☐ via tog() to reflect real state.
  -- ──
  for i, v in ipairs(items) do
    local k = v[2]
    if     k == "hdr_movement"  then pretick[i] = dropOpen.movement
    elseif k == "hdr_abilities" then pretick[i] = dropOpen.abilities
    elseif k == "hdr_weapons"   then pretick[i] = dropOpen.weapons
    elseif k == "hdr_defense"   then pretick[i] = dropOpen.defense
    elseif k == "hdr_targeting" then pretick[i] = dropOpen.targeting
    elseif k == "hdr_teleport"  then pretick[i] = dropOpen.teleport
    elseif k == "hdr_match"     then pretick[i] = dropOpen.match
    elseif k == "hdr_bots"      then pretick[i] = dropOpen.bots
    elseif k == "hdr_misc"      then pretick[i] = dropOpen.misc
    -- NOTE: mod keys are deliberately left as pretick[i] = false
    --       so they always open unchecked (auto-reset behavior).
    --       The ☑/☐ prefix in the label string shows the true toggle state.
    end
  end

  local result = gg.multiChoice(display, pretick, "⬜⬛ Made by SS-MODS ⬛⬜")
  if result == nil then return end

  -- ── Process selections ──
  -- For HEADERS: toggling the checkbox opens/closes the dropdown section.
  -- For MODS:    checking the box fires the mod (toggles it on or off),
  --              then the checkbox resets to unchecked next time menu opens.
  --              The ☑/☐ in the label reflects the actual enabled state.

  for i, v in ipairs(items) do
    local k   = v[2]
    local now = result[i] == true   -- was this item checked when OK was pressed?

    -- DROPDOWNS: only act if the tick changed from pretick
    if k == "hdr_movement" then
      if now ~= pretick[i] then dropOpen.movement  = now end

    elseif k == "hdr_abilities" then
      if now ~= pretick[i] then dropOpen.abilities = now end

    elseif k == "hdr_weapons" then
      if now ~= pretick[i] then dropOpen.weapons   = now end

    elseif k == "hdr_defense" then
      if now ~= pretick[i] then dropOpen.defense   = now end

    elseif k == "hdr_targeting" then
      if now ~= pretick[i] then dropOpen.targeting = now end

    elseif k == "hdr_teleport" then
      if now ~= pretick[i] then dropOpen.teleport  = now end

    elseif k == "hdr_match" then
      if now ~= pretick[i] then dropOpen.match     = now end

    elseif k == "hdr_bots" then
      if now ~= pretick[i] then dropOpen.bots      = now end

    elseif k == "hdr_misc" then
      if now ~= pretick[i] then dropOpen.misc      = now end

    -- MODS: fire only if the box was CHECKED (now == true).
    -- Checking toggles the mod on→off or off→on, then resets to unchecked.
    elseif now == true then
      -- MOVEMENT
      if     k == "speedHack"  then speedHackEnabled   = not speedHackEnabled;   if speedHackEnabled   then SpeedHack()    else FSpeedHack()    end
      elseif k == "gravity"    then gravityEnabled      = not gravityEnabled;      if gravityEnabled      then Gravity()      else FGravity()      end
      elseif k == "noclip"     then noclipEnabled       = not noclipEnabled;       if noclipEnabled       then Noclip()       else FNoclip()       end
      elseif k == "multiJump"  then multipleJumpEnabled = not multipleJumpEnabled; if multipleJumpEnabled then MultipleJump() else FMultipleJump() end
      elseif k == "noJump"     then noJumpEnabled       = not noJumpEnabled;       if noJumpEnabled       then NoJump()       else FNoJump()       end
      elseif k == "superJump"  then superJumpEnabled    = not superJumpEnabled;    if superJumpEnabled    then SuperJump()    else FSuperJump()    end
      -- ABILITIES
      elseif k == "skill1"        then skill1Enabled         = not skill1Enabled;         if skill1Enabled         then Skill1()         else FSkill1()         end
      elseif k == "durSkill1"     then durationSkill1Enabled = not durationSkill1Enabled; if durationSkill1Enabled then DurationSkill1()  else FDurationSkill1() end
      elseif k == "skill2"        then skill2Enabled         = not skill2Enabled;         if skill2Enabled         then Skill2()         else FSkill2()         end
      elseif k == "durSkill2"     then durationSkill2Enabled = not durationSkill2Enabled; if durationSkill2Enabled then DurationSkill2()  else FDurationSkill2() end
      elseif k == "infModule"     then infiniteModuleEnabled = not infiniteModuleEnabled; if infiniteModuleEnabled then InfiniteModule()  else FInfiniteModule() end
      elseif k == "titanCharge"   then titanChargeEnabled    = not titanChargeEnabled;    if titanChargeEnabled    then TitanCharge()    else FTitanCharge()    end
      elseif k == "energyRefresh" then energyRefreshEnabled  = not energyRefreshEnabled;  if energyRefreshEnabled  then EnergyRefresh()  else FEnergyRefresh()  end
      elseif k == "autoAbility1"    then autoAbility1Enabled    = not autoAbility1Enabled;    if autoAbility1Enabled    then AutoAbility1On()    else AutoAbilityOff(1)    end
      elseif k == "autoAbility2"    then autoAbility2Enabled    = not autoAbility2Enabled;    if autoAbility2Enabled    then AutoAbility2On()    else AutoAbilityOff(2)    end
      elseif k == "autoAbilityBoth" then autoAbilityBothEnabled = not autoAbilityBothEnabled; if autoAbilityBothEnabled then AutoAbilityBothOn() else AutoAbilityOff(3)    end
      -- WEAPONS
      elseif k == "rapidFire"   then rapidFireEnabled       = not rapidFireEnabled;       if rapidFireEnabled       then RapidFire()       else FRapidFire()       end
      elseif k == "autoFire"    then autoFireEnabled        = not autoFireEnabled;        if autoFireEnabled        then AutoFire()        else FAutoFire()        end
      elseif k == "weaponRange" then weaponRangeEnabled     = not weaponRangeEnabled;     if weaponRangeEnabled     then WeaponRange()     else FWeaponRange()     end
      elseif k == "noReload"    then noReloadEnabled        = not noReloadEnabled;        if noReloadEnabled        then NoReload()        else FNoReload()        end
      elseif k == "noRecoil"    then noRecoilEnabled        = not noRecoilEnabled;        if noRecoilEnabled        then NoRecoil()        else FNoRecoil()        end
      elseif k == "infAmmo"     then infiniteAmmoEnabled    = not infiniteAmmoEnabled;    if infiniteAmmoEnabled    then InfiniteAmmo()    else FInfiniteAmmo()    end
      elseif k == "damageMult"  then damageMultEnabled      = not damageMultEnabled;      if damageMultEnabled      then DamageMult()      else FDamageMult()      end
      elseif k == "bigRadius"   then bigRadiusEnabled       = not bigRadiusEnabled;       if bigRadiusEnabled       then BigRadius()       else FBigRadius()       end
      elseif k == "chargeRate"  then chargeRateEnabled      = not chargeRateEnabled;      if chargeRateEnabled      then ChargeRate()      else FChargeRate()      end
      elseif k == "accuracy"    then accuracyEnabled        = not accuracyEnabled;        if accuracyEnabled        then PerfectAccuracy() else FPerfectAccuracy() end
      elseif k == "projSpeed"   then projectileSpeedEnabled = not projectileSpeedEnabled; if projectileSpeedEnabled then ProjectileSpeed() else FProjectileSpeed() end
      elseif k == "turretSpeed" then turretSpeedEnabled     = not turretSpeedEnabled;     if turretSpeedEnabled     then TurretSpeed()     else FTurretSpeed()     end
      -- DEFENSE
      elseif k == "godMode"      then godModeEnabled        = not godModeEnabled;        if godModeEnabled        then GodMode()        else FGodMode()        end
      elseif k == "infHp"        then infiniteHpEnabled     = not infiniteHpEnabled;     if infiniteHpEnabled     then InfiniteHP()     else FInfiniteHP()     end
      elseif k == "maxHp"        then maxHpEnabled          = not maxHpEnabled;          if maxHpEnabled          then MaxHP()          else FMaxHP()          end
      elseif k == "antiDeath"    then antiDeathEnabled      = not antiDeathEnabled;      if antiDeathEnabled      then AntiDeath()      else FAntiDeath()      end
      elseif k == "infShield"    then infiniteShieldEnabled = not infiniteShieldEnabled; if infiniteShieldEnabled then InfiniteShield() else FInfiniteShield() end
      elseif k == "antiEMP"      then antiEMPEnabled        = not antiEMPEnabled;        if antiEMPEnabled        then AntiEMP()        else FAntiEMP()        end
      elseif k == "antiSkill"    then antiSkillEnabled      = not antiSkillEnabled;      if antiSkillEnabled      then AntiSkill()      else FAntiSkill()      end
      elseif k == "antiLockdown" then antiLockdownEnabled   = not antiLockdownEnabled;   if antiLockdownEnabled   then AntiLockdown()   else FAntiLockdown()   end
      elseif k == "antiBlind"    then antiBlindEnabled      = not antiBlindEnabled;      if antiBlindEnabled      then AntiBlind()      else FAntiBlind()      end
      -- TARGETING
      elseif k == "targetLock"  then targetLockEnabled  = not targetLockEnabled;  if targetLockEnabled  then TargetLock()  else FTargetLock()  end
      elseif k == "antiStealth" then antiStealthEnabled = not antiStealthEnabled; if antiStealthEnabled then AntiStealth() else FAntiStealth() end
      elseif k == "fov"         then fovEnabled         = not fovEnabled;         if fovEnabled         then WideFOV()     else FWideFOV()     end
      -- TELEPORT
      elseif k == "teleportBeacon" then teleportBeaconEnabled = not teleportBeaconEnabled; if teleportBeaconEnabled then TeleportBeacon() else FTeleportBeacon() end
      elseif k == "underground"    then undergroundEnabled    = not undergroundEnabled;    if undergroundEnabled    then Underground()    else FUnderground()    end
      -- MATCH
      elseif k == "matchTime" then matchTimeEnabled = not matchTimeEnabled; if matchTimeEnabled then FastMatchEnd() else FFastMatchEnd() end
      -- BOTS
      elseif k == "botsNoShoot" then botsDontShootEnabled = not botsDontShootEnabled; if botsDontShootEnabled then BotsDontShoot() else FBotsDontShoot() end
      elseif k == "shortDist"   then shortDistanceEnabled = not shortDistanceEnabled; if shortDistanceEnabled then ShortDistance() else FShortDistance() end
      -- MISC
      elseif k == "instBox"     then instantBoxEnabled     = not instantBoxEnabled;     if instantBoxEnabled     then InstantBox()     else FInstantBox()     end
      elseif k == "instAd"      then instantAdSkipEnabled  = not instantAdSkipEnabled;  if instantAdSkipEnabled  then InstantAdSkip()  else FInstantAdSkip()  end
      elseif k == "instUpgrade" then instantUpgradeEnabled = not instantUpgradeEnabled; if instantUpgradeEnabled then InstantUpgrade() else FInstantUpgrade() end
      -- UTIL
      elseif k == "help" then ShowHelp()
      elseif k == "exit" then Exit()
      end
    end
  end

  -- Reopen the menu so it stays up after every OK press
  HOME()
end

function ShowHelp()
  gg.alert(
    "⬜⬛ SS-MODS Feature Guide ⬛⬜\n\n" ..
    "⚡ MOVEMENT\n" ..
    "SpeedHack — Boosts robot movement speed\n" ..
    "Disable Gravity — Float in air, no falling\n" ..
    "Noclip — Walk through walls and terrain\n" ..
    "Multiple Jump — Jump infinite times mid-air\n" ..
    "No Jump — Disables all jumping\n" ..
    "Super Jump — Extreme vertical jump force\n\n" ..
    "⚔️ ABILITIES\n" ..
    "Infinite Skill 1/2 — Cooldown always zero\n" ..
    "No Duration Skill 1/2 — Active phase ends instantly\n" ..
    "Infinite Module — Module never goes on cooldown\n" ..
    "Instant Titan Charge — Titan always ready to deploy\n" ..
    "Instant Charge Weapons — Gauss/Weber charge instantly\n" ..
    "Fast Energy Refresh — Energy weapons recharge faster\n" ..
    "🔁 Auto Ability — Spam-fires both skills non-stop (uses SetPressedState + zero cooldown patches)\n\n" ..
    "🔫 WEAPONS\n" ..
    "Rapid Fire — Minimal shot interval (0.01s)\n" ..
    "Auto Fire — Near-zero delay between shots (0.01s)\n" ..
    "Weapon Range 1100m — Shoot from extreme distance\n" ..
    "No Reload — Shooting never locked by reload\n" ..
    "No Recoil — Removes all bullet spread\n" ..
    "Infinite Ammo — Ammo never decreases\n" ..
    "Damage Multiplier — Huge damage boost\n" ..
    "Big Radius Damage — AoE explosion radius x16\n" ..
    "Perfect Accuracy — Max accuracy multiplier\n" ..
    "Instant Hit Projectile — Bullets travel at extreme speed\n" ..
    "Max Turret Rotation Speed — Turret turns instantly\n\n" ..
    "🛡️ DEFENSE\n" ..
    "God Mode — Maximum damage resistance\n" ..
    "Infinite HP — HP cannot decrease\n" ..
    "Max HP Boost — Extreme health pool\n" ..
    "Anti Death — Mech cannot be destroyed\n" ..
    "Infinite Energy Shield — Shield HP always at max\n" ..
    "Anti EMP Block — EMP cannot lock abilities/modules\n" ..
    "Anti Skill Block — Pilot effects never blocked\n" ..
    "Anti Lockdown — Cannot be rooted or frozen\n" ..
    "Anti Blind — Vision never suppressed\n\n" ..
    "🎯 TARGETING\n" ..
    "Instant Target Lock — Lock on enemies immediately\n" ..
    "See Through Stealth — Always aim at stealthed targets\n" ..
    "Wide FOV — Expanded camera field of view\n\n" ..
    "🌀 TELEPORT\n" ..
    "Teleport to Beacon — Instantly teleport to nearest beacon\n" ..
    "Underground Mode — Sink below the map\n\n" ..
    "🕒 MATCH\n" ..
    "Fast Match End — Sets match time so game ends quickly\n\n" ..
    "🤖 BOTS / MAP\n" ..
    "Disable Bot AI — Bots stop attacking you\n" ..
    "Short Distance — Enemies always appear within 20m\n\n" ..
    "🎁 MISC\n" ..
    "Instant Box — Chest cooldown removed\n" ..
    "Instant Ad Skip — Skip ads and get reward instantly\n"
  )
end

function Exit()
  os.exit()
end

-- ══════════════════════════════════════
--  Shared HexPatch utility
-- ══════════════════════════════════════
local function makeHexPatch()
  local gg = gg
  local ti = gg.getTargetInfo()
  local arch = ti.x64
  local p_size = arch and 8 or 4
  local p_type = arch and 32 or 4

  local getvalue = function(address, flags)
    return gg.getValues({{address = address, flags = flags}})[1].value
  end

  local ptr = function(address)
    return getvalue(address, p_type)
  end

  local CString = function(address, str)
    local bytes = gg.bytes(str)
    for i = 1, #bytes do
      if getvalue(address + (i - 1), 1) & 255 ~= bytes[i] then
        return false
      end
    end
    return getvalue(address + #bytes, 1) == 0
  end

  local HexPatch = function(clazz, method, hex)
    local t, total = {}, 0
    gg.setRanges(-2080835)
    gg.clearResults()
    gg.searchNumber(string.format("Q 00 '%s' 00", method))
    if gg.getResultsCount() == 0 then return false end
    gg.refineNumber(method:byte(), 1)
    gg.searchPointer(0, p_type)
    local pointer_results = gg.getResults(gg.getResultsCount(), nil, nil, nil, nil, nil, p_type, nil,
      gg.POINTER_EXECUTABLE | gg.POINTER_EXECUTABLE_WRITABLE | gg.POINTER_WRITABLE | gg.POINTER_READ_ONLY)
    gg.clearResults()
    if #pointer_results == 0 then return false end
    for _, v in ipairs(pointer_results) do
      if CString(ptr(ptr(v.address + p_size) + p_size * 2), clazz) then
        for byte in string.gmatch(hex, "%S%S") do
          table.insert(t, {address = ptr(v.address - p_size * 2) + total, flags = 1, value = byte .. "r"})
          total = total + 1
        end
      end
    end
    if #t == 0 then return false end
    local res = gg.setValues(t)
    return type(res) ~= "string"
  end

  return HexPatch
end

-- ══════════════════════════════════════
--  MOVEMENT
-- ══════════════════════════════════════

function SpeedHack()
  local HexPatch = makeHexPatch()
  if HexPatch("MechStats", "get_Speed", "0000885280C3A8720000271EC0035FD6") then
    gg.toast("✔️ SpeedHack")
  else
    gg.toast("❌ SpeedHack Failed")
  end
end

function FSpeedHack()
  local HexPatch = makeHexPatch()
  if HexPatch("MechStats", "get_Speed", "08 24 46 29 28 01 08 4A 00 01 27 1E C0 03 5F D6") then
    gg.toast("❌ SpeedHack OFF")
  else
    gg.toast("❌ SpeedHack OFF Failed")
  end
end

function Gravity()
  local HexPatch = makeHexPatch()
  if HexPatch("Animator", "get_gravityWeight", "00008052C0035FD6") then
    gg.toast("✔️ Disable Gravity")
    return
  end
  if HexPatch("Animator", "get_gravityWeight", "C0035FD6") then
    gg.toast("✔️ Disable Gravity (method 2)")
    return
  end
  gg.toast("❌ Gravity Failed")
end

function FGravity()
  local HexPatch = makeHexPatch()
  if HexPatch("Animator", "get_gravityWeight", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Gravity Restored")
  else
    gg.toast("❌ Gravity Restore Failed")
  end
end

function Noclip()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("AnimMotion", "get_AnimationStateFlying", "20008052C0035FD6")
  if s1 then
    gg.toast("✔️ Noclip ON — robot floats freely")
  else
    gg.toast("❌ Noclip Failed")
  end
end

function FNoclip()
  local HexPatch = makeHexPatch()
  HexPatch("AnimMotion", "get_AnimationStateFlying", "FE0F1EF8F44F01A9")
  gg.toast("❌ Noclip OFF")
end

function MultipleJump()
  local HexPatch = makeHexPatch()
  if HexPatch("AnimJumping", "get_CanJump", "E003271EC0035FD6") then
    gg.toast("✔️ Multiple Jump")
  else
    gg.toast("❌ Multiple Jump Failed")
  end
end

function FMultipleJump()
  local HexPatch = makeHexPatch()
  if HexPatch("AnimJumping", "get_CanJump", "FE 0F 1E F8 F4 4F 01 A9") then
    gg.toast("❌ Multiple Jump OFF")
  else
    gg.toast("❌ Multiple Jump OFF Failed")
  end
end

function NoJump()
  local HexPatch = makeHexPatch()
  if HexPatch("AnimJumping", "Jump", "C0 03 5F D6") then
    gg.toast("✔️ No Jump")
  else
    gg.toast("❌ No Jump Failed")
  end
end

function FNoJump()
  local HexPatch = makeHexPatch()
  if HexPatch("AnimJumping", "Jump", "FE 0F 1D F8") then
    gg.toast("❌ No Jump OFF")
  else
    gg.toast("❌ No Jump OFF Failed")
  end
end

function SuperJump()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("AnimJumping", "get_CanJump", "E003271EC0035FD6")
  local s2 = HexPatch("AnimMotion", "get_SpeedFactor", "0000885280C3A8720000271EC0035FD6")
  if s1 or s2 then
    gg.toast("✔️ Super Jump")
  else
    gg.toast("❌ Super Jump Failed")
  end
end

function FSuperJump()
  local HexPatch = makeHexPatch()
  HexPatch("AnimJumping", "get_CanJump", "FE 0F 1E F8 F4 4F 01 A9")
  HexPatch("AnimMotion", "get_SpeedFactor", "FE0F1EF8F44F01A9")
  gg.toast("❌ Super Jump OFF")
end

-- ══════════════════════════════════════
--  ABILITIES
-- ══════════════════════════════════════

function Skill1()
  local HexPatch = makeHexPatch()
  local true_b  = "20008052C0035FD6"
  local false_b = "00008052C0035FD6"
  local zero_f  = "E003271EC0035FD6"

  local s1  = HexPatch("AbilityStateWithCooldownAndCharges", "get_IsActionAvailable", true_b)
  local s2  = HexPatch("AbilityStateWithCooldownAndCharges", "get_TimeRemaining",     zero_f)
  local s3  = HexPatch("AbilityStateWithCooldownAndCharges", "get_Cooldown",          zero_f)
  local s4  = HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable", true_b)
  local s5  = HexPatch("AbilityStateWithTimer",   "get_RemainingTime",     zero_f)
  local s6  = HexPatch("AbilityStateWithTimer",   "get_InitialTime",       zero_f)
  local s7  = HexPatch("AbilityStateWithDuration","get_IsActionAvailable", true_b)
  local s8  = HexPatch("AbilityStateWithDuration","get_TimeRemaining",     zero_f)
  local s9  = HexPatch("AbilityStateWaitForClick","get_IsActionAvailable", true_b)
  local s10 = HexPatch("AbilityStateWaitForClick","get_ClickAllowed",      true_b)
  local s11 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        false_b)
  local s12 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", false_b)
  local s13 = HexPatch("MechStats","get_Ability1Cooldown",           zero_f)
  local s14 = HexPatch("MechStats","get_Ability1CooldownAdditional", zero_f)

  local count = 0
  for _,v in ipairs({s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14}) do
    if v then count=count+1 end
  end
  if count > 0 then
    gg.toast("✔️ Infinite Skill 1 (" .. count .. " patches)")
  else
    gg.toast("❌ Skill 1 Failed")
  end
end

function FSkill1()
  local HexPatch = makeHexPatch()
  HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_InitialTime",      "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability1Cooldown",           "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability1CooldownAdditional", "FE0F1EF8F44F01A9")
  gg.toast("❌ Skill 1 OFF")
end

function DurationSkill1()
  local HexPatch = makeHexPatch()
  local true_b  = "20008052C0035FD6"
  local false_b = "00008052C0035FD6"
  local zero_f  = "E003271EC0035FD6"

  local s1  = HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable",true_b)
  local s2  = HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    zero_f)
  local s3  = HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         zero_f)
  local s4  = HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable",true_b)
  local s5  = HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    zero_f)
  local s6  = HexPatch("AbilityStateWithTimer",   "get_InitialTime",      zero_f)
  local s7  = HexPatch("AbilityStateWithDuration","get_IsActionAvailable",true_b)
  local s8  = HexPatch("AbilityStateWithDuration","get_TimeRemaining",    zero_f)
  local s9  = HexPatch("AbilityStateWithDuration","get_Duration",         zero_f)
  local s10 = HexPatch("AbilityStateWaitForClick","get_IsActionAvailable",true_b)
  local s11 = HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     true_b)
  local s12 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        false_b)
  local s13 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", false_b)
  local s14 = HexPatch("MechStats","get_Ability1Cooldown",           zero_f)
  local s15 = HexPatch("MechStats","get_Ability1CooldownAdditional", zero_f)
  local s16 = HexPatch("MechStats","get_Ability1Duration",           zero_f)
  local s17 = HexPatch("MechStats","get_Ability1DurationAdditional", zero_f)

  local count = 0
  for _,v in ipairs({s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17}) do
    if v then count=count+1 end
  end
  if count > 0 then
    gg.toast("✔️ No Duration Skill 1 (" .. count .. " patches)")
  else
    gg.toast("❌ Duration Skill 1 Failed")
  end
end

function FDurationSkill1()
  local HexPatch = makeHexPatch()
  HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_InitialTime",      "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_Duration",         "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability1Cooldown",           "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability1CooldownAdditional", "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability1Duration",           "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability1DurationAdditional", "FE0F1EF8F44F01A9")
  gg.toast("❌ Duration Skill 1 OFF")
end

function Skill2()
  local HexPatch = makeHexPatch()
  local true_b  = "20008052C0035FD6"
  local false_b = "00008052C0035FD6"
  local zero_f  = "E003271EC0035FD6"

  local s1  = HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable",true_b)
  local s2  = HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    zero_f)
  local s3  = HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         zero_f)
  local s4  = HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable",true_b)
  local s5  = HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    zero_f)
  local s6  = HexPatch("AbilityStateWithTimer",   "get_InitialTime",      zero_f)
  local s7  = HexPatch("AbilityStateWithDuration","get_IsActionAvailable",true_b)
  local s8  = HexPatch("AbilityStateWithDuration","get_TimeRemaining",    zero_f)
  local s9  = HexPatch("AbilityStateWaitForClick","get_IsActionAvailable",true_b)
  local s10 = HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     true_b)
  local s11 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        false_b)
  local s12 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", false_b)
  local s13 = HexPatch("MechStats","get_Ability2Cooldown",           zero_f)
  local s14 = HexPatch("MechStats","get_Ability2CooldownAdditional", zero_f)

  local count = 0
  for _,v in ipairs({s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14}) do
    if v then count=count+1 end
  end
  if count > 0 then
    gg.toast("✔️ Infinite Skill 2 (" .. count .. " patches)")
  else
    gg.toast("❌ Skill 2 Failed")
  end
end

function FSkill2()
  local HexPatch = makeHexPatch()
  HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_InitialTime",      "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability2Cooldown",           "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability2CooldownAdditional", "FE0F1EF8F44F01A9")
  gg.toast("❌ Skill 2 OFF")
end

function DurationSkill2()
  local HexPatch = makeHexPatch()
  local true_b  = "20008052C0035FD6"
  local false_b = "00008052C0035FD6"
  local zero_f  = "E003271EC0035FD6"

  local s1  = HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable",true_b)
  local s2  = HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    zero_f)
  local s3  = HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         zero_f)
  local s4  = HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable",true_b)
  local s5  = HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    zero_f)
  local s6  = HexPatch("AbilityStateWithTimer",   "get_InitialTime",      zero_f)
  local s7  = HexPatch("AbilityStateWithDuration","get_IsActionAvailable",true_b)
  local s8  = HexPatch("AbilityStateWithDuration","get_TimeRemaining",    zero_f)
  local s9  = HexPatch("AbilityStateWithDuration","get_Duration",         zero_f)
  local s10 = HexPatch("AbilityStateWaitForClick","get_IsActionAvailable",true_b)
  local s11 = HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     true_b)
  local s12 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        false_b)
  local s13 = HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", false_b)
  local s14 = HexPatch("MechStats","get_Ability2Cooldown",           zero_f)
  local s15 = HexPatch("MechStats","get_Ability2CooldownAdditional", zero_f)
  local s16 = HexPatch("MechStats","get_Ability2Duration",           zero_f)
  local s17 = HexPatch("MechStats","get_Ability2DurationAdditional", zero_f)

  local count = 0
  for _,v in ipairs({s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17}) do
    if v then count=count+1 end
  end
  if count > 0 then
    gg.toast("✔️ No Duration Skill 2 (" .. count .. " patches)")
  else
    gg.toast("❌ Duration Skill 2 Failed")
  end
end

function FDurationSkill2()
  local HexPatch = makeHexPatch()
  HexPatch("AbilityStateWithCooldownAndCharges","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithCooldownAndCharges","get_Cooldown",         "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_RemainingTime",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithTimer",   "get_InitialTime",      "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_TimeRemaining",    "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWithDuration","get_Duration",         "FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_IsActionAvailable","FE0F1EF8F44F01A9")
  HexPatch("AbilityStateWaitForClick","get_ClickAllowed",     "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByPilot",        "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable","get_AbilityLockedByStatusEffect", "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability2Cooldown",           "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability2CooldownAdditional", "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability2Duration",           "FE0F1EF8F44F01A9")
  HexPatch("MechStats","get_Ability2DurationAdditional", "FE0F1EF8F44F01A9")
  gg.toast("❌ Duration Skill 2 OFF")
end

function InfiniteModule()
  local HexPatch = makeHexPatch()
  local zero_f   = "E003271EC0035FD6"
  local ret_true = "20008052C0035FD6"

  local s1 = HexPatch("MechStats", "get_ModulePrice",           zero_f)
  local s2 = HexPatch("MechStats", "get_ModulePriceAdditional", zero_f)
  local s3 = HexPatch("AbilityPaymentAdapter", "CanPay",  ret_true)
  local s4 = HexPatch("AbilityPaymentAdapter", "TryPay",  "C0035FD6")
  local s5 = HexPatch("MatchWallet", "CanConsume", ret_true)
  local s6 = HexPatch("MatchWallet", "TryConsume", ret_true)
  local s7 = HexPatch("MechStats", "get_ModuleCooldown",           zero_f)
  local s8 = HexPatch("MechStats", "get_ModuleDuration",           zero_f)
  local s9 = HexPatch("MechStats", "get_ModuleCooldownMultiplier", zero_f)

  local count = 0
  for _, v in ipairs({s1,s2,s3,s4,s5,s6,s7,s8,s9}) do if v then count=count+1 end end
  if count > 0 then
    gg.toast("✔️ Infinite Module ON (" .. count .. " patches)")
  else
    gg.toast("❌ Infinite Module: patches failed — check process")
  end
end

function FInfiniteModule()
  local HexPatch = makeHexPatch()
  local orig = "FE0F1EF8F44F01A9"
  HexPatch("MechStats",            "get_ModulePrice",           orig)
  HexPatch("MechStats",            "get_ModulePriceAdditional", orig)
  HexPatch("AbilityPaymentAdapter","CanPay",                    orig)
  HexPatch("AbilityPaymentAdapter","TryPay",                    orig)
  HexPatch("MatchWallet",          "CanConsume",                orig)
  HexPatch("MatchWallet",          "TryConsume",                orig)
  HexPatch("MechStats",            "get_ModuleCooldown",        orig)
  HexPatch("MechStats",            "get_ModuleDuration",        orig)
  HexPatch("MechStats",            "get_ModuleCooldownMultiplier", orig)
  gg.toast("❌ Infinite Module OFF")
end

function TitanCharge()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("TitanChargeService", "get_Charged",     "20008052C0035FD6")
  local s2 = HexPatch("TitanChargeService", "get_TitanCharge", "E003B01EC0035FD6")
  if s1 or s2 then
    gg.toast("✔️ Instant Titan Charge")
  else
    gg.toast("❌ Titan Charge Failed")
  end
end

function FTitanCharge()
  local HexPatch = makeHexPatch()
  HexPatch("TitanChargeService", "get_Charged",     "FE0F1EF8F44F01A9")
  HexPatch("TitanChargeService", "get_TitanCharge", "FE0F1EF8F44F01A9")
  gg.toast("❌ Titan Charge OFF")
end

function ChargeRate()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_ChargeRateMultiplier", "00008052408FA8720000271EC0035FD6") then
    gg.toast("✔️ Instant Charge Weapons")
  else
    gg.toast("❌ Charge Rate Failed")
  end
end

function FChargeRate()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_ChargeRateMultiplier", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Charge Rate OFF")
  else
    gg.toast("❌ Charge Rate OFF Failed")
  end
end

function EnergyRefresh()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_EnergyRefreshRateMultiplier", "0000885280C3A8720000271EC0035FD6") then
    gg.toast("✔️ Fast Energy Refresh")
  else
    gg.toast("❌ Energy Refresh Failed")
  end
end

function FEnergyRefresh()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_EnergyRefreshRateMultiplier", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Energy Refresh OFF")
  else
    gg.toast("❌ Energy Refresh OFF Failed")
  end
end

-- ══════════════════════════════════════
--  WEAPONS
-- ══════════════════════════════════════

function RapidFire()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("EquipmentStats", "get_ShotIntervalMultiplier", "A099995280A9A7720000271EC0035FD6")
  local s2 = HexPatch("Weapon",         "get_ShotInterval",           "A099995280A9A7720000271EC0035FD6")
  local count = (s1 and 1 or 0)+(s2 and 1 or 0)
  if count > 0 then
    gg.toast("✔️ Rapid Fire (" .. count .. " patches)")
  else
    gg.toast("❌ Rapid Fire Failed")
  end
end

function FRapidFire()
  local HexPatch = makeHexPatch()
  HexPatch("EquipmentStats", "get_ShotIntervalMultiplier", "FE0F1EF8F44F01A9")
  HexPatch("Weapon",         "get_ShotInterval",           "FE0F1EF8F44F01A9")
  gg.toast("❌ Rapid Fire OFF")
end

function AutoFire()
  local HexPatch = makeHexPatch()
  if HexPatch("Weapon", "get_ShotInterval", "40E19A526084A7720000271EC0035FD6") then
    gg.toast("✔️ Auto Fire (0.01s)")
  else
    gg.toast("❌ Auto Fire Failed")
  end
end

function FAutoFire()
  local HexPatch = makeHexPatch()
  if HexPatch("Weapon", "get_ShotInterval", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Auto Fire OFF")
  else
    gg.toast("❌ Auto Fire OFF Failed")
  end
end

function WeaponRange()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_RangeMultiplier", "0000885280C3A8720000271EC0035FD6") then
    gg.toast("✔️ Weapon Range 1100m")
  else
    gg.toast("❌ Weapon Range Failed")
  end
end

function FWeaponRange()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_RangeMultiplier", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Weapon Range OFF")
  else
    gg.toast("❌ Weapon Range OFF Failed")
  end
end

function NoReload()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("Weapon", "get_isShootingLockedByReload",    "00008052C0035FD6")
  local s2 = HexPatch("Weapon", "get_IsShootingLockedByReload",    "00008052C0035FD6")
  local s3 = HexPatch("EquipmentStats", "get_ReloadAccelerationMultiplier", "A099995280A9A7720000271EC0035FD6")
  if s1 or s2 or s3 then
    local count = (s1 and 1 or 0)+(s2 and 1 or 0)+(s3 and 1 or 0)
    gg.toast("✔️ No Reload (" .. count .. " patches)")
  else
    gg.toast("❌ No Reload Failed")
  end
end

function FNoReload()
  local HexPatch = makeHexPatch()
  HexPatch("Weapon",         "get_isShootingLockedByReload",    "FE0F1EF8F44F01A9")
  HexPatch("Weapon",         "get_IsShootingLockedByReload",    "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats", "get_ReloadAccelerationMultiplier","FE0F1EF8F44F01A9")
  gg.toast("❌ No Reload OFF")
end

function NoRecoil()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_DispersionDeltaTimeMultiplier", "00008052C0035FD6") then
    gg.toast("✔️ No Recoil")
  else
    gg.toast("❌ No Recoil Failed")
  end
end

function FNoRecoil()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_DispersionDeltaTimeMultiplier", "FE0F1EF8F44F01A9") then
    gg.toast("❌ No Recoil OFF")
  else
    gg.toast("❌ No Recoil OFF Failed")
  end
end

function InfiniteAmmo()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("Weapon", "get_AmmoConsumptionPerShot", "00008052C0035FD6")
  local s2 = HexPatch("EquipmentStats", "get_ReloadAccelerationMultiplier", "A099995280A9A7720000271EC0035FD6")
  local count = (s1 and 1 or 0)+(s2 and 1 or 0)
  if count > 0 then
    gg.toast("✔️ Infinite Ammo (" .. count .. " patches)")
  else
    gg.toast("❌ Infinite Ammo Failed")
  end
end

function FInfiniteAmmo()
  local HexPatch = makeHexPatch()
  HexPatch("Weapon",         "get_AmmoConsumptionPerShot",       "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats", "get_ReloadAccelerationMultiplier", "FE0F1EF8F44F01A9")
  gg.toast("❌ Infinite Ammo OFF")
end

function DamageMult()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("WeaponBase", "get_Damage",                          "0000885280C3A8720000271EC0035FD6")
  local s2 = HexPatch("WeaponBase", "get_InternalDamageMultiplier",        "0000885280C3A8720000271EC0035FD6")
  local s3 = HexPatch("WeaponBase", "get_DamageResistPenetration",         "E003B01EC0035FD6")
  local s4 = HexPatch("EquipmentStats", "get_DamageResistPenetrationMultiplier", "E003B01EC0035FD6")
  local s5 = HexPatch("MechStats",  "get_DamageResistPenetration",         "E003B01EC0035FD6")
  local count = (s1 and 1 or 0)+(s2 and 1 or 0)+(s3 and 1 or 0)+(s4 and 1 or 0)+(s5 and 1 or 0)
  if count > 0 then
    gg.toast("✔️ Damage Multiplier (" .. count .. " patches)")
  else
    gg.toast("❌ Damage Multiplier Failed")
  end
end

function FDamageMult()
  local HexPatch = makeHexPatch()
  HexPatch("WeaponBase",    "get_Damage",                              "FE0F1EF8F44F01A9")
  HexPatch("WeaponBase",    "get_InternalDamageMultiplier",            "FE0F1EF8F44F01A9")
  HexPatch("WeaponBase",    "get_DamageResistPenetration",             "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats","get_DamageResistPenetrationMultiplier",   "FE0F1EF8F44F01A9")
  HexPatch("MechStats",     "get_DamageResistPenetration",             "FE0F1EF8F44F01A9")
  gg.toast("❌ Damage Multiplier OFF")
end

function BigRadius()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_AoeRadiusMultiplier", "0000905200038372C0035FD6") then
    gg.toast("✔️ Big Radius Damage (16x)")
  else
    gg.toast("❌ Big Radius Failed")
  end
end

function FBigRadius()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_AoeRadiusMultiplier", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Big Radius OFF")
  else
    gg.toast("❌ Big Radius OFF Failed")
  end
end

function PerfectAccuracy()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("EquipmentStats", "get_AccuracyMultiplier", "E003271EC0035FD6")
  local s2 = HexPatch("EquipmentStats", "get_AccuracyAdditional", "E003271EC0035FD6")
  if s1 or s2 then
    gg.toast("✔️ Perfect Accuracy")
  else
    gg.toast("❌ Perfect Accuracy Failed")
  end
end

function FPerfectAccuracy()
  local HexPatch = makeHexPatch()
  HexPatch("EquipmentStats", "get_AccuracyMultiplier", "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats", "get_AccuracyAdditional", "FE0F1EF8F44F01A9")
  gg.toast("❌ Perfect Accuracy OFF")
end

function ProjectileSpeed()
  local HexPatch = makeHexPatch()
  if HexPatch("Weapon", "get_ProjectileSpeed", "A086985266F88F72C0035FD6") then
    gg.toast("✔️ Instant Hit Projectile")
  else
    gg.toast("❌ Projectile Speed Failed")
  end
end

function FProjectileSpeed()
  local HexPatch = makeHexPatch()
  if HexPatch("Weapon", "get_ProjectileSpeed", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Projectile Speed OFF")
  else
    gg.toast("❌ Projectile Speed OFF Failed")
  end
end

function TurretSpeed()
  local HexPatch = makeHexPatch()
  if HexPatch("MechStats", "get_TurretRotationSpeedStackedMult", "E003271EC0035FD6") then
    gg.toast("✔️ Max Turret Rotation Speed")
  else
    gg.toast("❌ Turret Speed Failed")
  end
end

function FTurretSpeed()
  local HexPatch = makeHexPatch()
  if HexPatch("MechStats", "get_TurretRotationSpeedStackedMult", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Turret Speed OFF")
  else
    gg.toast("❌ Turret Speed OFF Failed")
  end
end

-- ══════════════════════════════════════
--  DEFENSE
-- ══════════════════════════════════════

function GodMode()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_DamageResistMultiplier", "0000803FC0035FD6") then
    gg.toast("✔️ God Mode")
  else
    gg.toast("❌ God Mode Failed")
  end
end

function FGodMode()
  local HexPatch = makeHexPatch()
  if HexPatch("EquipmentStats", "get_DamageResistMultiplier", "FE0F1EF8F44F01A9") then
    gg.toast("❌ God Mode OFF")
  else
    gg.toast("❌ God Mode OFF Failed")
  end
end

function InfiniteHP()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("MechStats", "get_HpMech", "0000885280C3A8720000271EC0035FD6")
  local s2 = HexPatch("DamageController", "get_DamageScale", "00008052C0035FD6")
  local s3 = HexPatch("DamageController", "get_HealScale", "0000885280C3A8720000271EC0035FD6")
  if s1 or s2 or s3 then
    gg.toast("✔️ Infinite HP")
  else
    gg.toast("❌ Infinite HP Failed")
  end
end

function FInfiniteHP()
  local HexPatch = makeHexPatch()
  HexPatch("MechStats", "get_HpMech", "FE0F1EF8F44F01A9")
  HexPatch("DamageController", "get_DamageScale", "FE0F1EF8F44F01A9")
  HexPatch("DamageController", "get_HealScale", "FE0F1EF8F44F01A9")
  gg.toast("❌ Infinite HP OFF")
end

function MaxHP()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("DamageController", "get_MaxHp", "0000885280C3A8720000271EC0035FD6")
  local s2 = HexPatch("MechStats", "get_HpMech", "0000885280C3A8720000271EC0035FD6")
  local s3 = HexPatch("DamageController", "get_ArmorPoints", "0000885280C3A8720000271EC0035FD6")
  if s1 or s2 or s3 then
    gg.toast("✔️ Max HP Boost")
  else
    gg.toast("❌ Max HP Failed")
  end
end

function FMaxHP()
  local HexPatch = makeHexPatch()
  HexPatch("DamageController", "get_MaxHp", "FE0F1EF8F44F01A9")
  HexPatch("MechStats", "get_HpMech", "FE0F1EF8F44F01A9")
  HexPatch("DamageController", "get_ArmorPoints", "FE0F1EF8F44F01A9")
  gg.toast("❌ Max HP OFF")
end

function AntiDeath()
  local HexPatch = makeHexPatch()
  if HexPatch("DamageController", "get_IsDestructed", "00008052C0035FD6") then
    gg.toast("✔️ Anti Death")
  else
    gg.toast("❌ Anti Death Failed")
  end
end

function FAntiDeath()
  local HexPatch = makeHexPatch()
  if HexPatch("DamageController", "get_IsDestructed", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Anti Death OFF")
  else
    gg.toast("❌ Anti Death OFF Failed")
  end
end

function InfiniteShield()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("MechStats", "get_HpEnergyShields",             "0000885280C3A8720000271EC0035FD6")
  local s2 = HexPatch("MechStats", "get_HpPhysicalShields",           "0000885280C3A8720000271EC0035FD6")
  local s3 = HexPatch("EquipmentStats", "get_HpEnergyShield",         "0000885280C3A8720000271EC0035FD6")
  local s4 = HexPatch("EquipmentStats", "get_HpEnergyShieldMultiplier","0000885280C3A8720000271EC0035FD6")
  local s5 = HexPatch("EquipmentStats", "get_HpPhysicalShield",       "0000885280C3A8720000271EC0035FD6")
  local count = (s1 and 1 or 0)+(s2 and 1 or 0)+(s3 and 1 or 0)+(s4 and 1 or 0)+(s5 and 1 or 0)
  if count > 0 then
    gg.toast("✔️ Infinite Energy Shield (" .. count .. " patches)")
  else
    gg.toast("❌ Infinite Shield Failed")
  end
end

function FInfiniteShield()
  local HexPatch = makeHexPatch()
  HexPatch("MechStats",     "get_HpEnergyShields",              "FE0F1EF8F44F01A9")
  HexPatch("MechStats",     "get_HpPhysicalShields",            "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats","get_HpEnergyShield",               "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats","get_HpEnergyShieldMultiplier",     "FE0F1EF8F44F01A9")
  HexPatch("EquipmentStats","get_HpPhysicalShield",             "FE0F1EF8F44F01A9")
  gg.toast("❌ Infinite Shield OFF")
end

function AntiEMP()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("MechStats", "get_AbilityLockedByStatusEffect", "C0 03 5F D6")
  local s2 = HexPatch("MechStats", "get_ModuleLockedByStatusEffect", "C0 03 5F D6")
  if s1 or s2 then
    gg.toast("✔️ Anti EMP Block")
  else
    gg.toast("❌ Anti EMP Failed")
  end
end

function FAntiEMP()
  local HexPatch = makeHexPatch()
  HexPatch("MechStats", "get_AbilityLockedByStatusEffect", "08 00 41 B9")
  HexPatch("MechStats", "get_ModuleLockedByStatusEffect", "08 10 41 B9")
  gg.toast("❌ Anti EMP OFF")
end

function AntiSkill()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("MechStats", "get_PilotEffectFeature1Enabled", "C0 03 5F D6")
  local s2 = HexPatch("MechStats", "get_PilotEffectFeature2Enabled", "C0 03 5F D6")
  if s1 or s2 then
    gg.toast("✔️ Anti Skill Block")
  else
    gg.toast("❌ Anti Skill Failed")
  end
end

function FAntiSkill()
  local HexPatch = makeHexPatch()
  HexPatch("MechStats", "get_PilotEffectFeature1Enabled", "08 18 41 B9")
  HexPatch("MechStats", "get_PilotEffectFeature2Enabled", "08 18 41 B9")
  gg.toast("❌ Anti Skill OFF")
end

function AntiLockdown()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("AnimMotion", "get_CanMove", "E003271EC0035FD6")
  local s2 = HexPatch("NetworkAbilityCustomizable", "get_AbilityLockedByPilot", "C0035FD6")
  if s1 or s2 then
    gg.toast("✔️ Anti Lockdown")
  else
    gg.toast("❌ Anti Lockdown Failed")
  end
end

function FAntiLockdown()
  local HexPatch = makeHexPatch()
  HexPatch("AnimMotion", "get_CanMove", "FE0F1EF8F44F01A9")
  HexPatch("NetworkAbilityCustomizable", "get_AbilityLockedByPilot", "FE0F1EF8F44F01A9")
  gg.toast("❌ Anti Lockdown OFF")
end

function AntiBlind()
  local HexPatch = makeHexPatch()
  if HexPatch("NetworkAbilityCustomizable", "get_AbilityLockedByStatusEffect", "C0035FD6") then
    gg.toast("✔️ Anti Blind")
  else
    gg.toast("❌ Anti Blind Failed")
  end
end

function FAntiBlind()
  local HexPatch = makeHexPatch()
  HexPatch("NetworkAbilityCustomizable", "get_AbilityLockedByStatusEffect", "FE0F1EF8F44F01A9")
  gg.toast("❌ Anti Blind OFF")
end

-- ══════════════════════════════════════
--  TARGETING
-- ══════════════════════════════════════

function TargetLock()
  local HexPatch = makeHexPatch()
  if HexPatch("MechStats", "get_TargetLockSpeed", "0000885280C3A8720000271EC0035FD6") then
    gg.toast("✔️ Instant Target Lock")
  else
    gg.toast("❌ Target Lock Failed")
  end
end

function FTargetLock()
  local HexPatch = makeHexPatch()
  if HexPatch("MechStats", "get_TargetLockSpeed", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Target Lock OFF")
  else
    gg.toast("❌ Target Lock OFF Failed")
  end
end

function AntiStealth()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("BaseCombatActor", "CanBeAimed", "20008052C0035FD6")
  local s2 = HexPatch("AntiStealthEffectHelper", "IsInAntiStealthRadius", "20008052C0035FD6")
  if s1 or s2 then
    gg.toast("✔️ See Through Stealth")
  else
    gg.toast("❌ Anti Stealth Failed")
  end
end

function FAntiStealth()
  local HexPatch = makeHexPatch()
  HexPatch("BaseCombatActor",        "CanBeAimed",               "FE0F1EF8F44F01A9")
  HexPatch("AntiStealthEffectHelper","IsInAntiStealthRadius",    "FE0F1EF8F44F01A9")
  gg.toast("❌ Anti Stealth OFF")
end

function WideFOV()
  local HexPatch = makeHexPatch()
  if HexPatch("Camera", "get_fieldOfView", "0000885280C3A8720000271EC0035FD6") then
    gg.toast("✔️ Wide FOV")
  else
    gg.toast("❌ Wide FOV Failed")
  end
end

function FWideFOV()
  local HexPatch = makeHexPatch()
  if HexPatch("Camera", "get_fieldOfView", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Wide FOV OFF")
  else
    gg.toast("❌ Wide FOV OFF Failed")
  end
end

-- ══════════════════════════════════════
--  TELEPORT NOT WORKING RIGHT NOW!!!!!!!!!!
-- ══════════════════════════════════════

function TeleportBeacon()
  local HexPatch = makeHexPatch()
  if HexPatch("TeleportationService", "StartTransition", "20008052C0035FD6") then
    gg.toast("✔️ Teleport to Beacon")
  else
    gg.toast("❌ Teleport Beacon Failed")
  end
end

function FTeleportBeacon()
  local HexPatch = makeHexPatch()
  if HexPatch("TeleportationService", "StartTransition", "FE0F1EF8F44F01A9") then
    gg.toast("❌ Teleport Beacon OFF")
  else
    gg.toast("❌ Teleport Beacon OFF Failed")
  end
end

function Underground()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("MechDynamics", "get_isGrounded", "00008052C0035FD6")
  local s2 = HexPatch("MechDynamics", "GetMechAltitude", "0000885280C3A8720000271EC0035FD6")
  if s1 or s2 then
    gg.toast("✔️ Underground Mode (enable Disable Gravity too)")
  else
    gg.toast("❌ Underground Failed")
  end
end

function FUnderground()
  local HexPatch = makeHexPatch()
  HexPatch("MechDynamics", "get_isGrounded", "FE0F1EF8F44F01A9")
  HexPatch("MechDynamics", "GetMechAltitude", "FE0F1EF8F44F01A9")
  gg.toast("❌ Underground OFF")
end

-- ══════════════════════════════════════
--  MATCH NOT WORKING RIGHT NOW!!!!!!!!!!
-- ══════════════════════════════════════

function FastMatchEnd()
  local HexPatch = makeHexPatch()
  local s1 = HexPatch("MatchTimeService", "get_MaxMatchDuration", "E003271EC0035FD6")
  local s2 = HexPatch("MatchTimeService", "get_MatchStartTime",   "E003271EC0035FD6")
  if s1 or s2 then
    gg.toast("✔️ Fast Match End")
  else
    gg.toast("❌ Fast Match End Failed")
  end
end

function FFastMatchEnd()
  local HexPatch = makeHexPatch()
  HexPatch("MatchTimeService", "get_MaxMatchDuration", "FE0F1EF8F44F01A9")
  HexPatch("MatchTimeService", "get_MatchStartTime",   "FE0F1EF8F44F01A9")
  gg.toast("❌ Fast Match End OFF")
end

-- ══════════════════════════════════════
--  BOTS / MAP
-- ══════════════════════════════════════

function BotsDontShoot()
  local HexPatch = makeHexPatch()
  if HexPatch("BotBehaviourAttackActive", "Start", "C0 03 5F D6") then
    gg.toast("✔️ Disable Bot AI")
  else
    gg.toast("❌ Disable Bot AI Failed")
  end
end

function FBotsDontShoot()
  local HexPatch = makeHexPatch()
  if HexPatch("BotBehaviourAttackActive", "Start", "FE 0F 1C F8") then
    gg.toast("❌ Disable Bot AI OFF")
  else
    gg.toast("❌ Disable Bot AI OFF Failed")
  end
end

function ShortDistance()
  local HexPatch = makeHexPatch()
  if HexPatch("TargetGroup", "get_distanceToTarget", "0090261EC0035FD6") then
    gg.toast("✔️ Short Distance (20M)")
  else
    gg.toast("❌ Short Distance Failed")
  end
end

function FShortDistance()
  local HexPatch = makeHexPatch()
  if HexPatch("TargetGroup", "get_distanceToTarget", "FE 4F BF A9 F3 03 00 AA") then
    gg.toast("❌ Short Distance OFF")
  else
    gg.toast("❌ Short Distance OFF Failed")
  end
end

-- ══════════════════════════════════════
--  MISC
-- ══════════════════════════════════════

function InstantBox()
  local HexPatch = makeHexPatch()
  if HexPatch("VideoAdTimerService", "GetChestCooldown", "00008052C0035FD6") then
    gg.toast("✔️ Instant Box")
  else
    gg.toast("❌ Instant Box Failed")
  end
end

function FInstantBox()
  local HexPatch = makeHexPatch()
  if HexPatch("VideoAdTimerService", "GetChestCooldown", "FE 0F 1E F8 F4 4F 01 A9") then
    gg.toast("❌ Instant Box OFF")
  else
    gg.toast("❌ Instant Box OFF Failed")
  end
end

function InstantAdSkip()
  local HexPatch = makeHexPatch()
  local results = 0
  local ret      = "C0035FD6"           -- bare ret (void methods — just return immediately)
  local false_b  = "00008052C0035FD6"   -- mov w0,#0 ; ret  (bool false / int 0)
  local true_b   = "20008052C0035FD6"   -- mov w0,#1 ; ret  (bool true)
  local big_i    = "E7030052C0035FD6"   -- mov w0,#999 ; ret

  -- ── Suppress the ad popup / video player from ever appearing ──
  -- VideoAdViewController::Show → return immediately without opening the player
  if HexPatch("VideoAdViewController",  "Show",                  ret)      then results = results + 1 end
  -- VideoAdViewController::ShowAd → same: stub out the "play ad video" call
  if HexPatch("VideoAdViewController",  "ShowAd",                ret)      then results = results + 1 end
  -- VideoAdService::ShowAd → the SDK-level "show an ad" call; NOP it
  if HexPatch("VideoAdService",         "ShowAd",                ret)      then results = results + 1 end
  -- VideoAdService::Show → alternate entrypoint
  if HexPatch("VideoAdService",         "Show",                  ret)      then results = results + 1 end

  -- ── Availability / gate bypasses (keep these so the reward button stays lit) ──
  -- AdCanBeShown() → true  (reward button always shows as available)
  if HexPatch("VideoAdStateService",    "AdCanBeShown",          true_b)   then results = results + 1 end
  -- IsAdLockByTimer() → false  (no cooldown)
  if HexPatch("VideoAdTimerService",    "IsAdLockByTimer",       false_b)  then results = results + 1 end
  -- IsAdsEnabled() → false  (bypass "ads must be enabled" gate)
  if HexPatch("VideoAdStateService",    "IsAdsEnabled",          false_b)  then results = results + 1 end
  -- BattlesLeft → 0  (removes "play a battle first" requirement)
  if HexPatch("VideoAdStateMessage",    "get_BattlesLeft",       false_b)  then results = results + 1 end
  -- DailyFreeAdUsed → 0  (reset daily counter)
  if HexPatch("VideoAdStateMessage",    "get_DailyFreeAdUsed",   false_b)  then results = results + 1 end
  -- DailyFreeAdTotal → 999  (inflate daily cap)
  if HexPatch("VideoAdStateMessage",    "get_DailyFreeAdTotal",  big_i)    then results = results + 1 end
  -- PremiumSkipAdUsed → true  (grant reward without watch flag)
  if HexPatch("VideoAdWatchedMessage",  "get_PremiumSkipAdUsed", true_b)   then results = results + 1 end

  -- ── Grant reward immediately: stub the "ad finished" callback to fire at once ──
  -- OnAdWatched / OnVideoAdWatched → return immediately (treated as instant-complete)
  if HexPatch("VideoAdStateService",    "OnAdWatched",           ret)      then results = results + 1 end
  if HexPatch("VideoAdStateService",    "OnVideoAdWatched",      ret)      then results = results + 1 end

  if results > 0 then
    gg.toast("✔️ No Ads — popup suppressed, reward instant (" .. results .. " patches)")
  else
    gg.toast("❌ No Ads Failed — ensure WR process is selected")
  end
end

function FInstantAdSkip()
  local HexPatch = makeHexPatch()
  local orig = "FE0F1EF8F44F01A9"
  HexPatch("VideoAdViewController",  "Show",                  orig)
  HexPatch("VideoAdViewController",  "ShowAd",                orig)
  HexPatch("VideoAdService",         "ShowAd",                orig)
  HexPatch("VideoAdService",         "Show",                  orig)
  HexPatch("VideoAdStateService",    "AdCanBeShown",          orig)
  HexPatch("VideoAdTimerService",    "IsAdLockByTimer",       orig)
  HexPatch("VideoAdStateService",    "IsAdsEnabled",          orig)
  HexPatch("VideoAdStateMessage",    "get_BattlesLeft",       orig)
  HexPatch("VideoAdStateMessage",    "get_DailyFreeAdUsed",   orig)
  HexPatch("VideoAdStateMessage",    "get_DailyFreeAdTotal",  orig)
  HexPatch("VideoAdWatchedMessage",  "get_PremiumSkipAdUsed", orig)
  HexPatch("VideoAdStateService",    "OnAdWatched",           orig)
  HexPatch("VideoAdStateService",    "OnVideoAdWatched",      orig)
  gg.toast("❌ No Ads OFF")
end

function InstantUpgrade()
  local HexPatch = makeHexPatch()
  local zero_i = "00008052C0035FD6"
  local zero_f = "E003271EC0035FD6"
  local true_b = "20008052C0035FD6"

  local s1  = HexPatch("EquipmentFullInfo", "get_OriginalUpgradeTime", zero_i)
  local s2  = HexPatch("EquipmentFullInfo", "get_FinalUpgradeTime",    zero_i)
  local s3  = HexPatch("EquipmentFullInfo", "get_UpgradeRestTimeSec",  zero_f)
  local s4  = HexPatch("EquipmentFullInfo", "get_NextUpgradeRank",     true_b)
  local s5  = HexPatch("MechFullInfo", "get_OriginalUpgradeTime", zero_i)
  local s6  = HexPatch("MechFullInfo", "get_FinalUpgradeTime",    zero_i)
  local s7  = HexPatch("MechFullInfo", "get_UpgradeRestTimeSec",  zero_f)
  local s8  = HexPatch("MechFullInfo", "get_NextUpgradeRank",     true_b)
  local s9  = HexPatch("ShipExtensionFullInfo", "get_OriginalUpgradeTime", zero_i)
  local s10 = HexPatch("ShipExtensionFullInfo", "get_FinalUpgradeTime",    zero_i)
  local s11 = HexPatch("ShipExtensionFullInfo", "get_UpgradeRestTimeSec",  zero_f)
  local s12 = HexPatch("ShipExtensionFullInfo", "get_NextUpgradeRank",     true_b)

  local count = 0
  for _, v in ipairs({s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12}) do
    if v then count = count + 1 end
  end
  if count > 0 then
    gg.toast("✔️ Instant Upgrade ON — press Enhance, then Collect immediately")
  else
    gg.toast("❌ Instant Upgrade Failed")
  end
end

function FInstantUpgrade()
  local HexPatch = makeHexPatch()
  local orig = "FE0F1EF8F44F01A9"
  HexPatch("EquipmentFullInfo",     "get_OriginalUpgradeTime", orig)
  HexPatch("EquipmentFullInfo",     "get_FinalUpgradeTime",    orig)
  HexPatch("EquipmentFullInfo",     "get_UpgradeRestTimeSec",  orig)
  HexPatch("EquipmentFullInfo",     "get_NextUpgradeRank",     orig)
  HexPatch("MechFullInfo",          "get_OriginalUpgradeTime", orig)
  HexPatch("MechFullInfo",          "get_FinalUpgradeTime",    orig)
  HexPatch("MechFullInfo",          "get_UpgradeRestTimeSec",  orig)
  HexPatch("MechFullInfo",          "get_NextUpgradeRank",     orig)
  HexPatch("ShipExtensionFullInfo", "get_OriginalUpgradeTime", orig)
  HexPatch("ShipExtensionFullInfo", "get_FinalUpgradeTime",    orig)
  HexPatch("ShipExtensionFullInfo", "get_UpgradeRestTimeSec",  orig)
  HexPatch("ShipExtensionFullInfo", "get_NextUpgradeRank",     orig)
  gg.toast("❌ Instant Upgrade OFF")
end

-- ══════════════════════════════════════
--  AUTO ABILITY
--  Three modes: Skill 1 only, Skill 2 only, Both.
--  Per-slot MechStats cooldowns patched separately so only the
--  requested slot is touched. Shared state-machine methods apply to all.
-- ══════════════════════════════════════

local function _autoAbilityApply(slot)
  local HexPatch = makeHexPatch()
  local zero_f  = "E003271EC0035FD6"
  local true_b  = "20008052C0035FD6"
  local false_b = "00008052C0035FD6"
  local count   = 0
  local function try(a,b,c) if HexPatch(a,b,c) then count=count+1 end end

  -- Simulate held button press (complete stub with epilogue)
  try("NetworkAbilityCustomizable", "SetPressedState",  "20008052C0035FD6")
  -- State machine: always advance, always reset to ready
  try("NetworkAbilityCustomizable", "GotoNextState",    false_b)
  try("NetworkAbilityCustomizable", "ChangeState",      false_b)

  -- Shared per-state-object getters (not slot-specific)
  try("AbilityStateWithCooldownAndCharges", "get_IsActionAvailable", true_b)
  try("AbilityStateWithCooldownAndCharges", "get_Cooldown",          zero_f)
  try("AbilityStateWithCooldownAndCharges", "get_TimeRemaining",     zero_f)
  try("AbilityStateWithTimer",    "get_IsActionAvailable",           true_b)
  try("AbilityStateWithTimer",    "get_RemainingTime",               zero_f)
  try("AbilityStateWithDuration", "get_IsActionAvailable",           true_b)
  try("AbilityStateWithDuration", "get_TimeRemaining",               zero_f)
  try("AbilityStateWaitForClick", "get_IsActionAvailable",           true_b)
  try("AbilityStateWaitForClick", "get_ClickAllowed",                true_b)
  try("NetworkAbilityCustomizable", "get_AbilityLockedByPilot",        false_b)
  try("NetworkAbilityCustomizable", "get_AbilityLockedByStatusEffect", false_b)

  -- Per-slot MechStats cooldowns — only patch the requested slot
  if slot == 1 or slot == 3 then
    try("MechStats", "get_Ability1Cooldown",           zero_f)
    try("MechStats", "get_Ability1CooldownAdditional", zero_f)
  end
  if slot == 2 or slot == 3 then
    try("MechStats", "get_Ability2Cooldown",           zero_f)
    try("MechStats", "get_Ability2CooldownAdditional", zero_f)
  end

  return count
end

function AutoAbility1On()
  local n = _autoAbilityApply(1)
  if n > 0 then gg.toast("✔️ Auto Ability 1 ON (" .. n .. " patches)")
  else           gg.toast("⚠️ Auto Ability 1 failed — select WR process") end
end

function AutoAbility2On()
  local n = _autoAbilityApply(2)
  if n > 0 then gg.toast("✔️ Auto Ability 2 ON (" .. n .. " patches)")
  else           gg.toast("⚠️ Auto Ability 2 failed — select WR process") end
end

function AutoAbilityBothOn()
  local n = _autoAbilityApply(3)
  if n > 0 then gg.toast("✔️ Auto Ability 1 & 2 ON (" .. n .. " patches)")
  else           gg.toast("⚠️ Auto Ability 1&2 failed — select WR process") end
end

function AutoAbilityOff(slot)
  local HexPatch = makeHexPatch()
  local orig = "FE0F1EF8F44F01A9"
  HexPatch("NetworkAbilityCustomizable", "SetPressedState",                  orig)
  HexPatch("NetworkAbilityCustomizable", "GotoNextState",                    orig)
  HexPatch("NetworkAbilityCustomizable", "ChangeState",                      orig)
  HexPatch("AbilityStateWithCooldownAndCharges", "get_IsActionAvailable",    orig)
  HexPatch("AbilityStateWithCooldownAndCharges", "get_Cooldown",             orig)
  HexPatch("AbilityStateWithCooldownAndCharges", "get_TimeRemaining",        orig)
  HexPatch("AbilityStateWithTimer",    "get_IsActionAvailable",              orig)
  HexPatch("AbilityStateWithTimer",    "get_RemainingTime",                  orig)
  HexPatch("AbilityStateWithDuration", "get_IsActionAvailable",              orig)
  HexPatch("AbilityStateWithDuration", "get_TimeRemaining",                  orig)
  HexPatch("AbilityStateWaitForClick", "get_IsActionAvailable",              orig)
  HexPatch("AbilityStateWaitForClick", "get_ClickAllowed",                   orig)
  HexPatch("NetworkAbilityCustomizable", "get_AbilityLockedByPilot",         orig)
  HexPatch("NetworkAbilityCustomizable", "get_AbilityLockedByStatusEffect",  orig)
  HexPatch("MechStats", "get_Ability1Cooldown",                              orig)
  HexPatch("MechStats", "get_Ability1CooldownAdditional",                    orig)
  HexPatch("MechStats", "get_Ability2Cooldown",                              orig)
  HexPatch("MechStats", "get_Ability2CooldownAdditional",                    orig)
  autoAbility1Enabled    = false
  autoAbility2Enabled    = false
  autoAbilityBothEnabled = false
  gg.toast("❌ Auto Ability OFF")
end

-- ══════════════════════════════════════
--  MAIN LOOP
-- ══════════════════════════════════════

while true do
  if gg.isVisible(false) then
    gg.setVisible(false)
    HOME()
  end
  gg.sleep(100)
end