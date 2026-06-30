#include "world/CampPlacementValidator.hpp"
#include "core/Constants.hpp"

namespace bunker
{

    bool CampPlacementValidator::validateAndSnap(const WorldGridState &grid, int &tileX, int &tileY) const
    {
        if (tileX < 1 || tileX >= Config::MAP_WIDTH - 1 || tileY < 1 || tileY >= Config::MAP_HEIGHT - 1)
            return false;

        // Если автомат прикрепления включен, ищем соседнюю стену для примагничивания
        if (m_AutoSnapEnabled && grid.sectorMap[tileX][tileY] == 0)
        {
            if (grid.sectorMap[tileX - 1][tileY] == 1)
                tileX = tileX - 1;
            else if (grid.sectorMap[tileX + 1][tileY] == 1)
                tileX = tileX + 1;
            else if (grid.sectorMap[tileX][tileY - 1] == 1)
                tileY = tileY - 1;
            else if (grid.sectorMap[tileX][tileY + 1] == 1)
                tileY = tileY + 1;
        }

        return true;
    }

} // namespace bunker