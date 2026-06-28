#include "world/StoryEventManager.hpp"
#include "core/Constants.hpp"
#include <iostream>

namespace bunker
{

    void StoryEventManager::evaluateZoneTriggers(GameState &gs, StoryFlagRegistry &flags)
    {
        // Триггер сектора Garage (Ангар техники Убежища 17)
        if (!flags.getFlag(StoryQuestFlag::GarageRepairUnlocked))
        {
            float gx = gs.playerPos.x - 13.0f;
            float gy = gs.playerPos.y - 12.0f;
            if (gx * gx + gy * gy < 9.0f) // Внутри радиуса ангара
            {
                flags.setFlag(StoryQuestFlag::GarageRepairUnlocked, true);
                gs.score += 500;
                std::cout << "[STORY EVENT] !! АНГАР ТЕХНИКИ ОБНАРУЖЕН !! Разблокирован крафт 'BT REPAIR KIT' в терминале!" << std::endl;
            }
        }

        // Триггер сектора CryoLocker (Крио-камеры консервации)
        if (!flags.getFlag(StoryQuestFlag::EnteredCryoLocker))
        {
            if (gs.playerPos.x < 4.0f && gs.playerPos.y < 4.0f)
            {
                flags.setFlag(StoryQuestFlag::EnteredCryoLocker, true);
                std::cout << "[STORY EVENT] Пилот вошёл в отсек Крио-консервации." << std::endl;
            }
        }
    }

} // namespace bunker