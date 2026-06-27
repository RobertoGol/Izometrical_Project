#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "ai/EnemySpawner.hpp"
#include "persistence/Inventory.hpp"

namespace bunker
{

    class WorldSession
    {
    public:
        WorldSession() = default;
        void generateDefaultWorld(GameState &gs, EnemySpawner &spawner);
        void update(GameState &gs, float dt);
        void interactWithContainers(GameState &gs, PlayerInventory &inventory);

    private:
        void spreadErosionToNeighbors(GameState &gs);
    };

} // namespace bunker
