#include "physics/PhysicsWorld.hpp"

#include <algorithm>

namespace bunker
{
    PhysicsWorld::PhysicsWorld(const GameState& gameState)
        : m_GameState(gameState)
    {
    }

    bool PhysicsWorld::isInsideWorld(int tileX, int tileY) const
    {
        return tileX >= 0 && tileX < Config::MAP_WIDTH && tileY >= 0 && tileY < Config::MAP_HEIGHT;
    }

    bool PhysicsWorld::isSolidTile(int tileX, int tileY) const
    {
        if (!isInsideWorld(tileX, tileY))
        {
            return true;
        }

        return m_GameState.sectorMap[tileX][tileY] == 1 && m_GameState.wallDurability[tileX][tileY] > 0;
    }

    bool PhysicsWorld::isCircleBlocked(float x, float y, float radius) const
    {
        const int minX = static_cast<int>(x - radius);
        const int maxX = static_cast<int>(x + radius);
        const int minY = static_cast<int>(y - radius);
        const int maxY = static_cast<int>(y + radius);

        for (int tileX = minX; tileX <= maxX; ++tileX)
        {
            for (int tileY = minY; tileY <= maxY; ++tileY)
            {
                if (isSolidTile(tileX, tileY))
                {
                    return true;
                }
            }
        }

        return false;
    }
} // namespace bunker
