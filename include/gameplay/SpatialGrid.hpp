#pragma once

#include "core/Types.hpp"
#include <vector>
#include <array>
#include <unordered_map>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Оптимизирующая пространственная сетка (Волна 9 / Broadphase Culling)
    // Разреживает O(N^2) проверки лучей видимости при рое в 100+ мобов
    // ═══════════════════════════════════════════════════════════════════════

    class SpatialGrid
    {
    private:
        static constexpr float CELL_SIZE = 4.0f;
        static constexpr int GRID_W = 16;
        static constexpr int GRID_H = 16;

        std::array<std::vector<std::size_t>, GRID_W * GRID_H> m_Buckets;

    public:
        SpatialGrid() = default;

        void clearBuckets();
        void registerEntity(std::size_t entityIndex, const Vector3D &pos);
        void queryRadius(const Vector3D &center, float radius, std::vector<std::size_t> &outIndices) const;

    private:
        int getBucketIndex(float worldX, float worldY) const;
    };

} // namespace bunker
