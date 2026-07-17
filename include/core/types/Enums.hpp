#pragma once

namespace bunker
{
    // ══════════════════════════════════════════════════════════════════════
    // Перечисления
    // ══════════════════════════════════════════════════════════════════════

    enum class AIState
    {
        Follow,
        Guard,
        Combat
    };
    enum class TankWeaponMode
    {
        Cannon,
        AutoCannon
    };
    enum class MissileStrikeMode
    {
        Ballistic,
        Artillery
    };
    enum class BulletType
    {
        Standard,
        Pellet,
        BallisticMissile,
        ArtilleryMissile
    };
    enum class UnitMode
    {
        Scout,
        Titan
    };

    enum class PilotClass
    {
        Grapple,
        Cloak,
        Stim,
        PhaseShift,
        HoloPilot,
        AWall,
        PulseBlade
    };

    enum class TitanClass
    {
        Ion,
        Scorch,
        Northstar,
        Ronin,
        Tone,
        Legion,
        Monarch
    };

    enum class ItemType
    {
        Weapon,
        Armor,
        Medicine,
        Ammo,
        Things,
        Quest,
        Resource
    };

    enum class CampObjectType
    {
        ConcreteWall,
        DefenseTurret,
        SupplyCrate
    };

    enum class LootContainerType
    {
        WoodenCrate,
        IronSafe,
        DevVault
    };

    enum class VehicleType
    {
        None,
        SteamCar,
        Motorcycle
    };


} // namespace bunker
