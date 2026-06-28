#include "gameplay/SpatialGrid.hpp"
#include <algorithm>
#include <cmath>

namespace bunker
{

    void SpatialGrid::clearBuckets()
    {
        for (auto &bucket : m_Buckets)
        {
            bucket.clear();
        }
    }

    void SpatialGrid::registerEntity(std::size_t entityIndex, const Vector3D &pos)
    {
        int idx = getBucketIndex(pos.x, pos.y);
        m_Buckets[idx].push_back(entityIndex);
    }

    void SpatialGrid::queryRadius(const Vector3D &center, float radius, std::vector<std::size_t> &outIndices) const
    {
        outIndices.clear();

        int minX = std::max(0, static_cast<int>((center.x - radius) / CELL_SIZE));
        int maxX = std::min(GRID_W - 1, static_cast<int>((center.x + radius) / CELL_SIZE));
        int minY = std::max(0, static_cast<int>((center.y - radius) / CELL_SIZE));
        int maxY = std::min(GRID_H - 1, static_cast<int>((center.y + radius) / CELL_SIZE));

        for (int bx = minX; bx <= maxX; ++bx)
        {
            for (int by = minY; by <= maxY; ++by)
            {
                int idx = by * GRID_W + bx;
                for (std::size_t eIdx : m_Buckets[idx])
                {
                    outIndices.push_back(eIdx);
                }
            }
        }
    }

    int SpatialGrid::getBucketIndex(float worldX, float worldY) const
    {
        int cx = std::clamp(static_cast<int>(worldX / CELL_SIZE), 0, GRID_W - 1);
        int cy = std::clamp(static_cast<int>(worldY / CELL_SIZE), 0, GRID_H - 1);
        return cy * GRID_W + cx;
    }

} // namespace bunker
