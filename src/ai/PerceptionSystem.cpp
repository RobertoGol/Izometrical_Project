#include "ai/PerceptionSystem.hpp"
#include "core/Constants.hpp"

namespace bunker
{

    bool PerceptionSystem::hasLineOfSight(const WorldGridState &grid, const Vector3D &from, const Vector3D &to)
    {
        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= 0.05f)
            return true;

        int steps = std::max(1, static_cast<int>(dist * 3.0f));
        float stepX = dx / static_cast<float>(steps);
        float stepY = dy / static_cast<float>(steps);

        float currX = from.x;
        float currY = from.y;

        for (int i = 1; i < steps; ++i)
        {
            currX += stepX;
            currY += stepY;

            int tileX = static_cast<int>(currX);
            int tileY = static_cast<int>(currY);

            if (tileX >= 0 && tileX < Config::MAP_WIDTH && tileY >= 0 && tileY < Config::MAP_HEIGHT)
            {
                // Если луч зрения натыкается на стену (тайл = 1)
                if (grid.sectorMap[tileX][tileY] == 1)
                {
                    return false; // Видимость заблокирована преградой
                }
            }
        }

        return true; // Прямая видимость установлена
    }

    float PerceptionSystem::calculateEffectiveDetectRadius(float baseRadius, float visibilityModifier)
    {
        return baseRadius * std::clamp(visibilityModifier, 0.20f, 1.0f);
    }

} // namespace bunker