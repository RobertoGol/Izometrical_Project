#pragma once

#include "core/Constants.hpp"
#include "gameplay/GameState.hpp"

namespace bunker
{
    class PhysicsWorld
    {
      public:
        explicit PhysicsWorld(const GameState& gameState);

        bool isInsideWorld(int tileX, int tileY) const;
        bool isSolidTile(int tileX, int tileY) const;
        bool isCircleBlocked(float x, float y, float radius) const;

      private:
        const GameState& m_GameState;
    };
} // namespace bunker
