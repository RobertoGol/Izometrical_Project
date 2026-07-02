#include "persistence/ModularEquipmentSystem.hpp"
#include "core/Types.hpp"

namespace bunker
{

    ModularEquipmentSystem::ModularEquipmentSystem()
    {
        initializeDatabase();
    }

    void ModularEquipmentSystem::initializeDatabase()
    {
        // 1) Каноничная База Брони, Экзоскелетов и Одежды из Вики-баз (Log Horizon + Titanfall + Fallout)
        m_ArmorRegistry.push_back({1001, "LH_GEVAUDAN", "Log Horizon: Majeste du Gevaudan Leather Armor", ArmorClassSlot::Chest, 24.0f, 28.0f, 0.50f, 1.20f, "Shadow Wolf Raid Leather (William Massachusetts)"});
        m_ArmorRegistry.push_back({1002, "LH_DRAGONFLY", "Log Horizon: Dragonfly Bead of Cataclysmic Change", ArmorClassSlot::Head, 18.0f, 35.0f, 0.75f, 1.10f, "Kaguya Dialogue Talisman (Soujiro Seta)"});
        m_ArmorRegistry.push_back({1003, "TF_STIM_SIM", "Titanfall 2: Stim Simulacrum Pilot Rig", ArmorClassSlot::ExoRig, 35.0f, 35.0f, 0.30f, 1.35f, "IMC +35% Locomotion Speed Boost Engine"});
        m_ArmorRegistry.push_back({1004, "TF_GRAPPLE", "Titanfall 2: Grapple Tactical Pilot Suit", ArmorClassSlot::ExoRig, 30.0f, 30.0f, 0.25f, 1.25f, "Frontier Tower Parkour & Hook System"});
        m_ArmorRegistry.push_back({1005, "FO76_SECRET", "Fallout 76: Secret Service Jet Plating Torso", ArmorClassSlot::Chest, 65.0f, 65.0f, 0.80f, 1.15f, "Pre-War Appalachia Wastelanders Jetpack Mod"});
        m_ArmorRegistry.push_back({1006, "FONV_DRAGOON", "Fallout NV: Chinese Crimson Dragoon Stealth Suit", ArmorClassSlot::Chest, 25.0f, 40.0f, 0.40f, 1.15f, "Sub-level 1C Hoover Infiltrator (+5 Sneak)"});
        m_ArmorRegistry.push_back({1007, "FONV_GHOST", "Fallout NV: Ranger Vest Red Scarf Outfit", ArmorClassSlot::Chest, 38.0f, 42.0f, 0.35f, 1.08f, "Mojave Outpost Sniper Ghost Patrol Tier"});
        // 2) Рецепты верстака крафта РобКо
        m_Recipes.push_back({1001, "Adventurer Mana Cloak", ItemType::Armor, WorkstationType::ArmorWorkbench, 4.5f, 25, 4, 10});
        m_Recipes.push_back({1002, "Pulse Pilot Parkour Rig", ItemType::Armor, WorkstationType::ArmorWorkbench, 5.0f, 40, 8, 25});
        m_Recipes.push_back({1003, "Vault 17 Combat Plating", ItemType::Armor, WorkstationType::ArmorWorkbench, 6.0f, 50, 10, 0});
        m_Recipes.push_back({1004, "Ranger Recon Duster", ItemType::Armor, WorkstationType::ArmorWorkbench, 4.8f, 35, 6, 15});
        m_Recipes.push_back({1005, "Secret Service Jet Rig", ItemType::Armor, WorkstationType::ArmorWorkbench, 8.5f, 100, 20, 50});
    }

    CraftResult ModularEquipmentSystem::craftItem(PlayerInventory &inv, int recipeIndex)
    {
        if (recipeIndex < 0 || recipeIndex >= static_cast<int>(m_Recipes.size()))
            return CraftResult::InvalidRecipeIndex;

        const auto &r = m_Recipes[recipeIndex];

        // ── Фаза 1: только проверка, никаких списаний ──
        if (r.requiredScrap > 0 && !inv.hasItem(CraftItemIDs::ScrapMetal, r.requiredScrap))
            return CraftResult::InsufficientScrap;

        if (r.requiredCircuits > 0 && !inv.hasItem(CraftItemIDs::Circuits, r.requiredCircuits))
            return CraftResult::InsufficientCircuits;

        if (r.requiredCoreEnergy > 0 && !inv.hasItem(CraftItemIDs::CoreEnergyCell, r.requiredCoreEnergy))
            return CraftResult::InsufficientCoreEnergy;

        // ── Фаза 2: все проверки прошли — теперь безопасно списывать ──
        if (r.requiredScrap > 0)
            inv.removeItem(CraftItemIDs::ScrapMetal, r.requiredScrap);

        if (r.requiredCircuits > 0)
            inv.removeItem(CraftItemIDs::Circuits, r.requiredCircuits);

        if (r.requiredCoreEnergy > 0)
            inv.removeItem(CraftItemIDs::CoreEnergyCell, r.requiredCoreEnergy);

        inv.addItem(r.resultItemID, r.resultType, 1, r.resultWeight, r.resultName);
        return CraftResult::Success;
    }

    void ModularEquipmentSystem::applyPlatingDamage(ModularTankChassis &tank, const Vector3D &hitDir, float damage)
    {
        // Определение части танка по направлению удара (Канон: модули БТ-7274)
        if (hitDir.y > 0.5f)
        {
            float actualDmg = damage * (1.0f - tank.frontalArmor.damageReductionThreshold);
            tank.frontalArmor.currentHp = std::max(0.0f, tank.frontalArmor.currentHp - actualDmg);
            std::cout << "[TITAN PART] Попадание в лобовой бастион! Остаток щита: " << tank.frontalArmor.currentHp << " HP." << std::endl;
        }
        else if (hitDir.y < -0.5f)
        {
            float actualDmg = damage * (1.0f - tank.rearArmor.damageReductionThreshold);
            tank.rearArmor.currentHp = std::max(0.0f, tank.rearArmor.currentHp - actualDmg);
            std::cout << "[TITAN PART] Критический пробой в корму танка! Остаток кормы: " << tank.rearArmor.currentHp << " HP." << std::endl;
        }
        else if (hitDir.x < 0.0f)
        {
            tank.leftTrack.currentHp = std::max(0.0f, tank.leftTrack.currentHp - damage);
            std::cout << "[TITAN PART] Повреждена левая гусеница! HP: " << tank.leftTrack.currentHp << std::endl;
        }
        else
        {
            tank.rightTrack.currentHp = std::max(0.0f, tank.rightTrack.currentHp - damage);
            std::cout << "[TITAN PART] Повреждена правая гусеница! HP: " << tank.rightTrack.currentHp << std::endl;
        }

        if (tank.isMobilityCrippled())
        {
            std::cout << "[TITAN ALARM] ХОДОВАЯ ЧАСТЬ КРИТИЧЕСКИ ПОВРЕЖДЕНА! Мобильность БТ-7274 упала на 60%!" << std::endl;
        }
    }

    std::vector<CraftingRecipe> ModularEquipmentSystem::getRecipesForStation(WorkstationType stationType) const
    {
        std::vector<CraftingRecipe> filtered;

        for (const auto &recipe : m_Recipes)
        {
            if (recipe.stationType == stationType)
                filtered.push_back(recipe);
        }

        return filtered;
    }

} // namespace bunker