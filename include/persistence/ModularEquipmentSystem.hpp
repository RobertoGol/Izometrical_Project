#pragma once

#include "core/Types.hpp"
#include "content/Workstations.hpp"
#include "gameplay/GameState.hpp"
#include "persistence/Inventory.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // Реестр Одежды, Брони и Оружия (Log Horizon + Titanfall 1/2 + Fallout 4/76/NV)
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class ArmorClassSlot
    {
        Head,
        Chest,
        Legs,
        ExoRig
    };

    struct ArmorItemDef
    {
        int itemID = 0;
        std::string codename;
        std::string displayName;
        ArmorClassSlot slot = ArmorClassSlot::Chest;
        float ballisticResist = 10.0f;
        float energyResist = 10.0f;
        float etherErosionResist = 0.0f;
        float movementSpeedBonus = 1.0f;
        std::string loreOrigin = "Vault 17 RobCo Standard";
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // Модульная Архитектура Танка БТ-7274 (Разделение на физические части)
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class TankModuleStatus
    {
        Unbreakable,
        Optimal,
        Damaged,
        Crippled
    };

    struct TankPartPlating
    {
        std::string partName = "Hull";
        float currentHp = 200.0f;
        float maxHp = 200.0f;
        float damageReductionThreshold = 0.5f;

        TankModuleStatus status() const
        {
            if (currentHp <= 0.0f)
                return TankModuleStatus::Crippled;
            if (currentHp <= maxHp * 0.4f)
                return TankModuleStatus::Damaged;
            return TankModuleStatus::Optimal;
        }
    };

    struct ModularTankChassis
    {
        TankPartPlating leftTrack = {"Left Track", 150.0f, 150.0f, 0.3f};
        TankPartPlating rightTrack = {"Right Track", 150.0f, 150.0f, 0.3f};
        TankPartPlating coreReactor = {"Core Reactor (350 Bar / 180C)", 300.0f, 300.0f, 0.7f};
        TankPartPlating mainCannon = {"Main Ordnance Cannon", 180.0f, 180.0f, 0.4f};
        TankPartPlating frontalArmor = {"Frontal Plating Shield", 400.0f, 400.0f, 0.85f};
        TankPartPlating rearArmor = {"Rear Engine Plating", 120.0f, 120.0f, 0.15f};

        bool isMobilityCrippled() const { return leftTrack.status() == TankModuleStatus::Crippled || rightTrack.status() == TankModuleStatus::Crippled; }
        bool isCannonBroken() const { return mainCannon.status() == TankModuleStatus::Crippled; }
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // Верстак Крафта РобКо / Log Horizon Synthesis Bench
    // ═══════════════════════════════════════════════════════════════════════════════

    struct CraftingRecipe
    {
        int resultItemID = 0;
        std::string resultName;
        ItemType resultType = ItemType::Armor;
        WorkstationType stationType = WorkstationType::ArmorWorkbench;
        float resultWeight = 4.5f;

        int requiredScrap = 10;
        int requiredCircuits = 2;
        int requiredCoreEnergy = 0;
    };

    class ModularEquipmentSystem
    {
    public:
        ModularEquipmentSystem();

        void initializeDatabase();
        bool craftItem(GameState &gs, PlayerInventory &inv, int recipeIndex);
        void applyPlatingDamage(ModularTankChassis &tank, const Vector3D &hitDir, float damage);

        std::vector<CraftingRecipe> getRecipesForStation(WorkstationType stationType) const;
        const std::vector<ArmorItemDef> &armorDatabase() const { return m_ArmorRegistry; }
        const std::vector<CraftingRecipe> &recipes() const { return m_Recipes; }
        ModularTankChassis &titanRuntimeChassis() { return m_ActiveTank; }

    private:
        std::vector<ArmorItemDef> m_ArmorRegistry;
        std::vector<CraftingRecipe> m_Recipes;
        ModularTankChassis m_ActiveTank;
    };

} // namespace bunker
