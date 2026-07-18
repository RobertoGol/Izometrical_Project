#pragma once

#include "gameplay/advanced/AdvancedMechanicsCommon.hpp"
#include "world/CampPlacementValidator.hpp"
namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 9) C.A.M.P. BUILDING
    // ═══════════════════════════════════════════════════════════════════════════════

    struct CampObject
    {
        int id = 0;
        CampObjectType type = CampObjectType::ConcreteWall;
        int tileX = 0;
        int tileY = 0;
        float health = 100.0f;
    };

    class CampSystem
    {
      public:
        static constexpr unsigned int ITEM_BUILD_MATERIAL = 201;

        void toggle()
        {
            m_Enabled = !m_Enabled;
        }
        bool enabled() const
        {
            return m_Enabled;
        }

        void cycleType();
        void updatePreview(const GameState& gs, const Vector3D& mouseWorld, const CampPlacementValidator& validator);
        bool place(GameState& gs, PlayerInventory& inv, float costMult = 1.0f);
        void updateTurrets(GameState& gs, float dt);

        const CampPreview& preview() const
        {
            return m_Preview;
        }
        const std::vector<CampObject>& objects() const
        {
            return m_Objects;
        }

      private:
        bool m_Enabled = false;
        int m_NextId = 0;
        CampPreview m_Preview;
        std::vector<CampObject> m_Objects;

        static int baseCost(CampObjectType type);
        bool canPlace(const GameState& gs, int x, int y) const;
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 10) SPATIAL GRID TEMPLATE (Legacy Generic Culling Container)
    // ═══════════════════════════════════════════════════════════════════════════════

    template <typename T>
    class TemplateSpatialGrid
    {
      public:
        explicit TemplateSpatialGrid(float cellSize = 2.0f) : m_CellSize(std::max(0.25f, cellSize)) {}

        void clear()
        {
            m_Cells.clear();
        }

        void insert(Vector3D pos, T value)
        {
            m_Cells[key(cellX(pos.x), cellY(pos.y))].push_back(value);
        }

        template <typename Fn>
        void query(Vector3D pos, float radius, Fn fn) const
        {
            const int minX = cellX(pos.x - radius);
            const int maxX = cellX(pos.x + radius);
            const int minY = cellY(pos.y - radius);
            const int maxY = cellY(pos.y + radius);
            for (int x = minX; x <= maxX; ++x)
            {
                for (int y = minY; y <= maxY; ++y)
                {
                    auto it = m_Cells.find(key(x, y));
                    if (it == m_Cells.end())
                    {
                        continue;
                    }
                    for (const auto& value : it->second)
                    {
                        fn(value);
                    }
                }
            }
        }

      private:
        float m_CellSize;
        std::unordered_map<std::int64_t, std::vector<T>> m_Cells;

        int cellX(float x) const
        {
            return static_cast<int>(std::floor(x / m_CellSize));
        }
        int cellY(float y) const
        {
            return static_cast<int>(std::floor(y / m_CellSize));
        }
        static std::int64_t key(int x, int y)
        {
            return (static_cast<std::int64_t>(x) << 32) ^ static_cast<unsigned int>(y);
        }
    };


} // namespace bunker
