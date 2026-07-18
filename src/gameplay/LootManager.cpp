#include "gameplay/LootManager.hpp"

#include "gameplay/advanced/AdvancedMechanicsSurvivalSystems.hpp"

#include <utility>

namespace bunker
{
    LootManager::LootManager()
        : m_Rng(0x17A55E7u)
    {
        addDefaults();
    }

    InventoryItem LootManager::roll(LootTier tier)
    {
        auto& table = m_Tables[tier];
        if (table.empty())
        {
            return {1, ItemType::Resource, 1, 0.1f, "SCRAP"};
        }

        float total = 0.0f;
        for (const auto& entry : table)
        {
            total += entry.weight;
        }

        std::uniform_real_distribution<float> pick(0.0f, total);
        float remaining = pick(m_Rng);
        for (const auto& entry : table)
        {
            remaining -= entry.weight;
            if (remaining <= 0.0f)
            {
                InventoryItem item = entry.item;
                std::uniform_int_distribution<int> quantity(entry.minQty, entry.maxQty);
                item.quantity = quantity(m_Rng);
                return item;
            }
        }

        return table.back().item;
    }

    void LootManager::fillContainer(LootContainer& container, LootTier tier, int rolls)
    {
        container.containsItems.clear();
        for (int i = 0; i < rolls; ++i)
        {
            container.containsItems.push_back(roll(tier));
        }

        if (container.type == LootContainerType::DevVault)
        {
            container.containsItems.push_back({1001, ItemType::Quest, 1, 15.0f, "TANK CORE BT (ANALOG TITAN CORE)"});
        }
    }

    void LootManager::normalizeWorldLoot(GameState& gameState)
    {
        int containerIndex = 0;
        for (auto& container : gameState.lootContainers)
        {
            if (container.isOpened || !container.containsItems.empty())
            {
                continue;
            }

            fillContainer(container, tierForContainer(container), 1 + (containerIndex++ % 3));
        }
    }

    LootTier LootManager::tierForContainer(const LootContainer& container) const
    {
        switch (container.type)
        {
        case LootContainerType::WoodenCrate:
            return LootTier::Common;
        case LootContainerType::IronSafe:
            return LootTier::Rare;
        case LootContainerType::DevVault:
            return LootTier::Legendary;
        default:
            return LootTier::Common;
        }
    }

    void LootManager::add(LootTier tier,
                          unsigned int id,
                          ItemType type,
                          std::string name,
                          int minQ,
                          int maxQ,
                          float weight,
                          float unitWeight)
    {
        m_Tables[tier].push_back({{id, type, 1, unitWeight, std::move(name)}, minQ, maxQ, weight});
    }

    void LootManager::addDefaults()
    {
        add(LootTier::Common, 201, ItemType::Resource, "SCRAP METAL", 2, 8, 8.0f, 0.08f);
        add(LootTier::Common, SurvivalSystem::ITEM_AMMO_556, ItemType::Ammo, "5.56 AMMO BOX", 1, 3, 6.0f, 0.15f);
        add(LootTier::Common, SurvivalSystem::ITEM_RATION_PROTEIN, ItemType::Medicine, "PROTEIN RATION", 1, 2, 3.0f,
            0.25f);
        add(LootTier::Uncommon, SurvivalSystem::ITEM_STIM, ItemType::Medicine, "STIM INJECTOR", 1, 2, 5.0f, 0.10f);
        add(LootTier::Uncommon, SurvivalSystem::ITEM_RATION_STAMINA, ItemType::Medicine, "STAMINA RATION", 1, 2, 4.0f,
            0.25f);
        add(LootTier::Rare, TankUtilitySystem::ITEM_REPAIR_KIT, ItemType::Resource, "BT REPAIR KIT", 1, 2, 4.0f, 0.60f);
        add(LootTier::Rare, SurvivalSystem::ITEM_RATION_FOCUS, ItemType::Medicine, "FOCUS RATION", 1, 1, 3.0f, 0.20f);
        add(LootTier::Epic, 777, ItemType::Weapon, "GMOD TOOLGUN", 1, 1, 1.0f, 0.0f);
        add(LootTier::Legendary, 999, ItemType::Quest, "DEV DEBUG BACKPACK", 1, 1, 1.0f, 0.0f);
        add(LootTier::Legendary, 888, ItemType::Weapon, "DEBUGGUN CHAIN LIGHTNING", 1, 1, 1.0f, 0.0f);
        add(LootTier::Uncommon, 520, ItemType::Medicine, "SUGAR FREE NUKA SHINE", 1, 2, 4.0f, 0.20f);
        add(LootTier::Rare, 521, ItemType::Medicine, "WEIGHT BE-GONE POTION", 1, 1, 3.0f, 0.10f);
        add(LootTier::Epic, 710, ItemType::Weapon, "SHEEPSQUATCH QUILL CLUB", 1, 1, 2.0f, 4.5f);
        add(LootTier::Legendary, 810, ItemType::Quest, "SPACE EXPLORER BACKPACK", 1, 1, 1.0f, 1.0f);
        add(LootTier::Rare, 811, ItemType::Resource, "PIONEER SCOUT BADGE", 1, 1, 4.0f, 0.01f);
    }
} // namespace bunker
