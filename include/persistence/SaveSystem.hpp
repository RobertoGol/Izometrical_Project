#pragma once

#include "core/Types.hpp"
#include "core/ECS.hpp"
#include "gameplay/GameState.hpp"
#include "persistence/Inventory.hpp"
#include "core/Constants.hpp"
#include <string>

namespace bunker
{

    class SaveSystem
    {
    public:
        SaveSystem() = default;

        static bool writeSave(unsigned int slot, const GameState& gs, const PlayerInventory& inventory,
                              const Registry* registry = nullptr);
        static bool readSave(unsigned int slot, GameState& gs, PlayerInventory& inventory, Registry* registry = nullptr);
        static bool saveExists(unsigned int slot);
    };

} // namespace bunker
