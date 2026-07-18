#include "gameplay/LootManager.hpp"

#include "engine/Log.hpp"
#include "gameplay/advanced/AdvancedMechanicsSurvivalSystems.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace bunker
{
    LootManager::LootManager()
        : m_Rng(0x17A55E7u)
    {
        addDefaults();
    }

    namespace
    {
        std::string trim(std::string value)
        {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
            {
                return {};
            }
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }

        std::vector<std::string> splitCsv(const std::string& line)
        {
            std::vector<std::string> parts;
            std::stringstream stream(line);
            std::string part;
            while (std::getline(stream, part, ','))
            {
                parts.push_back(trim(part));
            }
            return parts;
        }

        LootTier parseTier(const std::string& value)
        {
            if (value == "Uncommon")
                return LootTier::Uncommon;
            if (value == "Rare")
                return LootTier::Rare;
            if (value == "Epic")
                return LootTier::Epic;
            if (value == "Legendary")
                return LootTier::Legendary;
            return LootTier::Common;
        }

        ItemType parseItemType(const std::string& value)
        {
            if (value == "Weapon")
                return ItemType::Weapon;
            if (value == "Armor")
                return ItemType::Armor;
            if (value == "Medicine")
                return ItemType::Medicine;
            if (value == "Ammo")
                return ItemType::Ammo;
            if (value == "Quest")
                return ItemType::Quest;
            if (value == "Things")
                return ItemType::Things;
            return ItemType::Resource;
        }

        bool parseLootRow(const std::vector<std::string>& parts,
                          unsigned int& id,
                          int& minQty,
                          int& maxQty,
                          float& weight,
                          float& unitWeight)
        {
            try
            {
                id = static_cast<unsigned int>(std::stoul(parts[2]));
                minQty = std::stoi(parts[5]);
                maxQty = std::stoi(parts[6]);
                weight = std::stof(parts[7]);
                unitWeight = std::stof(parts[8]);
            }
            catch (...)
            {
                return false;
            }

            return minQty > 0 && maxQty >= minQty && weight > 0.0f && unitWeight >= 0.0f;
        }
    } // namespace

    InventoryItem LootManager::rollFromTable(std::vector<LootRollEntry>& table)
    {
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

    InventoryItem LootManager::roll(LootTier tier)
    {
        return rollFromTable(m_Tables[tier]);
    }

    InventoryItem LootManager::rollForRegion(const std::string& regionId, LootTier tier)
    {
        auto region = m_RegionTables.find(regionId);
        if (region == m_RegionTables.end())
        {
            return roll(tier);
        }

        auto table = region->second.find(tier);
        if (table == region->second.end() || table->second.empty())
        {
            return roll(tier);
        }

        return rollFromTable(table->second);
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

    void LootManager::fillContainerForRegion(LootContainer& container,
                                             const std::string& regionId,
                                             LootTier tier,
                                             int rolls)
    {
        container.containsItems.clear();
        for (int i = 0; i < rolls; ++i)
        {
            container.containsItems.push_back(rollForRegion(regionId, tier));
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

            fillContainerForRegion(
                container, gameState.mapMeta.currentMapName, tierForContainer(container), 1 + (containerIndex++ % 3));
            if (container.respawnDelaySeconds <= 0.0f)
            {
                container.respawnDelaySeconds = 300.0f;
            }
        }
    }

    void LootManager::updateRespawns(GameState& gameState, float dt)
    {
        for (auto& container : gameState.lootContainers)
        {
            if (!container.isOpened || container.respawnDelaySeconds <= 0.0f)
            {
                continue;
            }

            container.respawnTimerSeconds += dt;
            if (container.respawnTimerSeconds < container.respawnDelaySeconds)
            {
                continue;
            }

            container.isOpened = false;
            container.respawnTimerSeconds = 0.0f;
            fillContainerForRegion(container, gameState.mapMeta.currentMapName, tierForContainer(container), 1);
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

    void LootManager::addRegional(const std::string& regionId,
                                  LootTier tier,
                                  unsigned int id,
                                  ItemType type,
                                  std::string name,
                                  int minQ,
                                  int maxQ,
                                  float weight,
                                  float unitWeight)
    {
        m_RegionTables[regionId][tier].push_back({{id, type, 1, unitWeight, std::move(name)}, minQ, maxQ, weight});
    }

    void LootManager::loadExternalTables(const std::string& directory)
    {
        namespace fs = std::filesystem;
        m_RegionTables.clear();

        const fs::path root(directory);
        if (!fs::exists(root))
        {
            bunker::logInfo() << "[LOOT] External loot directory not found: " << directory << std::endl;
            return;
        }

        for (const fs::directory_entry& entry : fs::directory_iterator(root))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".loot")
            {
                continue;
            }

            std::ifstream file(entry.path());
            std::string line;
            while (std::getline(file, line))
            {
                line = trim(line);
                if (line.empty() || line[0] == '#')
                {
                    continue;
                }

                const std::vector<std::string> parts = splitCsv(line);
                if (parts.size() < 9)
                {
                    bunker::logInfo() << "[LOOT] Skipping malformed loot row in " << entry.path().string() << "."
                                      << std::endl;
                    continue;
                }

                unsigned int id = 0;
                int minQty = 0;
                int maxQty = 0;
                float weight = 0.0f;
                float unitWeight = 0.0f;
                if (!parseLootRow(parts, id, minQty, maxQty, weight, unitWeight))
                {
                    bunker::logInfo() << "[LOOT] Skipping invalid loot row in " << entry.path().string() << "."
                                      << std::endl;
                    continue;
                }

                addRegional(parts[0], parseTier(parts[1]), id, parseItemType(parts[3]), parts[4], minQty, maxQty,
                            weight, unitWeight);
            }
        }

        bunker::logInfo() << "[LOOT] External region loot tables loaded: " << m_RegionTables.size() << std::endl;
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
