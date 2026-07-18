#include "gameplay/AdvancedMechanics.hpp"

#include <algorithm>
#include <cmath>

namespace bunker
{

    void TankUtilitySystem::update(GameState& gs, float dt)
    {
        for (auto& s : m_Sparks)
        {
            s.pos += s.vel * dt;
            s.ttl -= dt;
        }
        m_Sparks.erase(
            std::remove_if(m_Sparks.begin(), m_Sparks.end(), [](const MuzzleSpark& s) { return s.ttl <= 0.0f; }),
            m_Sparks.end());

        m_Runtime.utilityCooldown = std::max(0.0f, m_Runtime.utilityCooldown - dt);
        m_Runtime.cannonThermalLoad = advClamp(m_Runtime.cannonThermalLoad - 18.0f * dt, 0.0f, 100.0f);
        if (m_Runtime.cannonThermalLoad < 62.0f)
        {
            m_Runtime.overheated = false;
        }

        if (gs.titan.isPiloted)
        {
            gs.playerPos = gs.titan.position;
            gs.playerMode = UnitMode::Titan;
        }

        if (m_Runtime.towing)
        {
            const Vector3D toTank = gs.titan.position - m_Runtime.towAnchor;
            m_Runtime.towAnchor += toTank * advClamp(dt * 1.8f, 0.0f, 1.0f);
            if (toTank.lengthSq() < 0.35f)
            {
                m_Runtime.towing = false;
            }
        }
    }

    void TankUtilitySystem::nextUtility()
    {
        switch (m_Runtime.utility)
        {
        case TankUtilityMode::BucketRig:
            m_Runtime.utility = TankUtilityMode::RamShield;
            break;
        case TankUtilityMode::RamShield:
            m_Runtime.utility = TankUtilityMode::TowCoupler;
            break;
        default:
            m_Runtime.utility = TankUtilityMode::BucketRig;
            break;
        }
    }

    void TankUtilitySystem::swapSeat()
    {
        m_Runtime.seat = (m_Runtime.seat == TankSeat::Driver) ? TankSeat::Gunner : TankSeat::Driver;
    }

    bool TankUtilitySystem::useUtility(GameState& gs)
    {
        if (m_Runtime.utilityCooldown > 0.0f)
        {
            return false;
        }
        switch (m_Runtime.utility)
        {
        case TankUtilityMode::BucketRig:
            carveFront(gs, 2, 35);
            m_Runtime.utilityCooldown = 1.0f;
            return true;
        case TankUtilityMode::RamShield:
            gs.titan.systems.tracksCondition = advClamp(gs.titan.systems.tracksCondition + 12.0f, 0.0f, 100.0f);
            gs.titan.health = advClamp(gs.titan.health + 10.0f, 0.0f, gs.titan.maxHealth);
            for (auto& e : gs.enemies)
            {
                if (e.isAlive && advDistSq(e.position, gs.titan.position) < 2.0f * 2.0f)
                {
                    if (DamageSystem::applyEnemyDamage(gs, e, 60.0f, DamageType::Kinetic))
                    {
                        gs.score += 20;
                    }
                }
            }
            m_Runtime.utilityCooldown = 3.0f;
            return true;
        case TankUtilityMode::TowCoupler:
            m_Runtime.towing = true;
            m_Runtime.towAnchor = gs.mouseWorldPos;
            m_Runtime.utilityCooldown = 2.0f;
            return true;
        default:
            break;
        }
        return false;
    }

    bool TankUtilitySystem::registerCannonShot(GameState& gs, float heat)
    {
        if (m_Runtime.overheated)
        {
            return false;
        }
        m_Runtime.cannonThermalLoad = advClamp(m_Runtime.cannonThermalLoad + heat, 0.0f, 100.0f);
        if (m_Runtime.cannonThermalLoad >= 100.0f)
        {
            m_Runtime.overheated = true;
            gs.titan.systems.turretStatus = advClamp(gs.titan.systems.turretStatus - 4.0f, 0.0f, 100.0f);
        }

        for (int i = 0; i < 4; ++i)
        {
            const float ang = static_cast<float>(i) * 1.57f;
            const Vector3D vel{std::cos(ang) * 6.5f, std::sin(ang) * 6.5f, 2.0f};
            m_Sparks.push_back({gs.titan.position, vel, 0.12f});
        }
        return true;
    }

    bool TankUtilitySystem::repairInHangar(GameState& gs, PlayerInventory& inv)
    {
        if (!isInsideHangar(gs.titan.position))
        {
            return false;
        }
        if (!inv.removeItem(ITEM_REPAIR_KIT, 1))
        {
            return false;
        }
        gs.titan.health = advClamp(gs.titan.health + 70.0f, 0.0f, gs.titan.maxHealth);
        gs.titan.systems.coreEnergy = advClamp(gs.titan.systems.coreEnergy + 25.0f, 0.0f, 100.0f);
        gs.titan.systems.sensorLink = advClamp(gs.titan.systems.sensorLink + 35.0f, 0.0f, 100.0f);
        gs.titan.systems.tracksCondition = advClamp(gs.titan.systems.tracksCondition + 35.0f, 0.0f, 100.0f);
        gs.titan.systems.turretStatus = advClamp(gs.titan.systems.turretStatus + 35.0f, 0.0f, 100.0f);
        return true;
    }

    void TankUtilitySystem::carveFront(GameState& gs, int radius, int damage)
    {
        const int cx = static_cast<int>(std::round(gs.titan.position.x));
        const int cy = static_cast<int>(std::round(gs.titan.position.y));
        for (int x = cx - radius; x <= cx + radius; ++x)
        {
            for (int y = cy - radius; y <= cy + radius; ++y)
            {
                if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
                {
                    continue;
                }
                if (gs.sectorMap[x][y] == 1)
                {
                    gs.wallDurability[x][y] -= damage;
                    if (gs.wallDurability[x][y] <= 0)
                    {
                        gs.sectorMap[x][y] = 0;
                    }
                }
            }
        }
    }

} // namespace bunker
