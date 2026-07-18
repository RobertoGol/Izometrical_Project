#pragma once

#include "core/ECS.hpp"
#include "gameplay/GameState.hpp"
#include "persistence/Inventory.hpp"

namespace bunker
{
    class AdvancedMechanics;
    class DoorTransition;
    class ModularEquipmentSystem;

    struct SaveGameContext
    {
        unsigned int slot = 1;
        GameState* gameState = nullptr;
        PlayerInventory* inventory = nullptr;
        Registry* registry = nullptr;
        AdvancedMechanics* advanced = nullptr;
        DoorTransition* doorTransition = nullptr;
        ModularEquipmentSystem* modularEquipment = nullptr;
    };

    class SaveGame
    {
      public:
        static bool exists(unsigned int slot);
        static bool save(const SaveGameContext& context);
        static bool load(const SaveGameContext& context);
    };
} // namespace bunker
