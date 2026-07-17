#pragma once

#include "gameplay/GameState.hpp"
#include "persistence/StoryFlagRegistry.hpp"

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Менеджер сюжетных зон и событий Убежища 17 (Волна 6)
    // При входе в Garage разблокирует крафт ремкомплектов БТ в терминале ангара
    // ═══════════════════════════════════════════════════════════════════════

    class StoryEventManager
    {
    public:
        StoryEventManager() = default;

        static void evaluateZoneTriggers(GameState &gs, StoryFlagRegistry &flags);
    };

} // namespace bunker