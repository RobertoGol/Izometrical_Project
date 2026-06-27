#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "engine/TimeShift.hpp"
#include "ai/HostileAISystem.hpp"
#include <SFML/Graphics.hpp>

namespace bunker
{

    struct AdvancedMechanics;

    class GameRenderer
    {
    public:
        static void renderFloor(sf::RenderWindow &window, const GameState &gs, const TimeShift &timeShift);
        static void renderEntities(sf::RenderWindow &window, const GameState &gs, const TimeShift &timeShift, const HostileAISystem &hostileAI);
        static void renderAdvancedWorld(sf::RenderWindow &window, const AdvancedMechanics &adv);
        static void renderAdvancedHUD(sf::RenderWindow &window, const AdvancedMechanics &adv, const sf::Font *font);
    };

} // namespace bunker
