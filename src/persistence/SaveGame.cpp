#include "persistence/SaveGame.hpp"

#include "persistence/SaveSystem.hpp"

namespace bunker
{
    bool SaveGame::exists(unsigned int slot)
    {
        return SaveSystem::saveExists(slot);
    }

    bool SaveGame::save(const SaveGameContext& context)
    {
        if (context.gameState == nullptr || context.inventory == nullptr)
        {
            return false;
        }

        return SaveSystem::writeSave(
            context.slot, *context.gameState, *context.inventory, context.registry, context.advanced);
    }

    bool SaveGame::load(const SaveGameContext& context)
    {
        if (context.gameState == nullptr || context.inventory == nullptr)
        {
            return false;
        }

        return SaveSystem::readSave(
            context.slot, *context.gameState, *context.inventory, context.registry, context.advanced);
    }
} // namespace bunker
