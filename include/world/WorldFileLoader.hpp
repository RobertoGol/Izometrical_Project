#pragma once

#include "gameplay/GameState.hpp"

#include <string>

namespace bunker
{
    class DoorTransition;

    class WorldFileLoader
    {
      public:
        static bool load(const std::string& path, GameState& gameState, DoorTransition* doorTransition = nullptr);
        static std::string pathForWorldId(const std::string& worldId);
    };
} // namespace bunker
