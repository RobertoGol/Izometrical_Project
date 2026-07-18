#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 5) REACTIVE WORLD SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    // ═══════════════════════════════════════════════════════════════════════════════
    // 9) CAMP SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    void CampSystem::cycleType()
    {
        switch (m_Preview.activeType)
        {
        case CampObjectType::ConcreteWall:
            m_Preview.activeType = CampObjectType::DefenseTurret;
            break;
        case CampObjectType::DefenseTurret:
            m_Preview.activeType = CampObjectType::SupplyCrate;
            break;
        default:
            m_Preview.activeType = CampObjectType::ConcreteWall;
            break;
        }
    }

    void CampSystem::updatePreview(const GameState& gs,
                                   const Vector3D& mouseWorld,
                                   const CampPlacementValidator& validator)
    {
        int tileX = static_cast<int>(std::floor(mouseWorld.x));
        int tileY = static_cast<int>(std::floor(mouseWorld.y));
        const bool validatorAccepted = validator.validateAndSnap(gs, tileX, tileY);

        m_Preview.tileX = tileX;
        m_Preview.tileY = tileY;
        if (!validatorAccepted)
        {
            m_Preview.isPlacementValid = false;
            return;
        }

        m_Preview.isPlacementValid = canPlace(gs, m_Preview.tileX, m_Preview.tileY);
    }

    bool CampSystem::place(GameState& gs, PlayerInventory& inv, float costMult)
    {
        if (!m_Enabled || !m_Preview.isPlacementValid)
        {
            return false;
        }
        const int cost = std::max(1, static_cast<int>(baseCost(m_Preview.activeType) * costMult));
        if (!inv.removeItem(ITEM_BUILD_MATERIAL, cost))
        {
            return false;
        }

        CampObject obj;
        obj.id = ++m_NextId;
        obj.type = m_Preview.activeType;
        obj.tileX = m_Preview.tileX;
        obj.tileY = m_Preview.tileY;
        obj.health = (obj.type == CampObjectType::ConcreteWall)
                         ? 180.0f
                         : (obj.type == CampObjectType::DefenseTurret ? 90.0f : 60.0f);
        m_Objects.push_back(obj);

        switch (obj.type)
        {
        case CampObjectType::ConcreteWall:
            gs.sectorMap[obj.tileX][obj.tileY] = 1;
            gs.wallDurability[obj.tileX][obj.tileY] = 180;
            break;
        case CampObjectType::SupplyCrate:
        {
            LootContainer c;
            c.position = {obj.tileX + 0.5f, obj.tileY + 0.5f, 0.0f};
            c.type = LootContainerType::WoodenCrate;
            gs.lootContainers.push_back(c);
            break;
        }
        default:
            break;
        }
        return true;
    }

    void CampSystem::updateTurrets(GameState& gs, float dt)
    {
        (void)dt;
        for (const auto& obj : m_Objects)
        {
            if (obj.type != CampObjectType::DefenseTurret)
            {
                continue;
            }
            Vector3D pos{obj.tileX + 0.5f, obj.tileY + 0.5f, 0.0f};
            Enemy* best = nullptr;
            float bestD = 5.5f * 5.5f;
            for (auto& e : gs.enemies)
            {
                if (!e.isAlive)
                {
                    continue;
                }
                const float d = advDistSq(pos, e.position);
                if (d < bestD)
                {
                    bestD = d;
                    best = &e;
                }
            }
            if (best)
            {
                DamageSystem::applyEnemyDamage(gs, *best, 12.0f * gs.deltaTime, DamageType::Kinetic);
            }
        }
    }

    int CampSystem::baseCost(CampObjectType type)
    {
        switch (type)
        {
        case CampObjectType::ConcreteWall:
            return 4;
        case CampObjectType::DefenseTurret:
            return 12;
        case CampObjectType::SupplyCrate:
            return 6;
        default:
            break;
        }
        return 5;
    }

    bool CampSystem::canPlace(const GameState& gs, int x, int y) const
    {
        if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
        {
            return false;
        }
        if (gs.sectorMap[x][y] == 1)
        {
            return false;
        }
        if (advDistSq({x + 0.5f, y + 0.5f, 0.0f}, gs.playerPos) > 5.0f * 5.0f)
        {
            return false;
        }
        for (const auto& obj : m_Objects)
        {
            if (obj.tileX == x && obj.tileY == y)
            {
                return false;
            }
        }
        return true;
    }

    void CampSystem::restoreObjects(const std::vector<CampObject>& objects)
    {
        m_Objects = objects;
        m_NextId = 0;
        for (const auto& object : m_Objects)
        {
            m_NextId = std::max(m_NextId, object.id);
        }
    }

} // namespace bunker
