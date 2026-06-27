#pragma once

#include "core/Types.hpp"
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

        static bool writeSave(unsigned int slot, const GameState &gs, const PlayerInventory &inventory);
        static bool readSave(unsigned int slot, GameState &gs, PlayerInventory &inventory);
        static bool saveExists(unsigned int slot);
    };

} // namespace bunker
