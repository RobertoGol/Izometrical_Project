#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"

#include <map>
#include <random>
#include <string>
#include <vector>

namespace bunker
{
    enum class LootTier
    {
        Common,
        Uncommon,
        Rare,
        Epic,
        Legendary
    };

    struct LootRollEntry
    {
        InventoryItem item;
        int minQty = 1;
        int maxQty = 1;
        float weight = 1.0f;
    };

    class LootManager
    {
      public:
        LootManager();

        InventoryItem roll(LootTier tier);
        void fillContainer(LootContainer& container, LootTier tier, int rolls);
        void normalizeWorldLoot(GameState& gameState);
        LootTier tierForContainer(const LootContainer& container) const;

      private:
        std::mt19937 m_Rng;
        std::map<LootTier, std::vector<LootRollEntry>> m_Tables;

        void add(LootTier tier, unsigned int id, ItemType type, std::string name, int minQ, int maxQ, float weight,
                 float unitWeight);
        void addDefaults();
    };

    class LootGenerator : public LootManager
    {
      public:
        LootGenerator() = default;
    };
} // namespace bunker
