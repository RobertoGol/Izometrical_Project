#include "world/DestructibleEnvironment.hpp"
#include <iostream>
#include <cmath>

namespace bunker
{

    DestructibleEnvironmentManager::DestructibleEnvironmentManager()
    {
        spawnDefaultProps();
    }

    void DestructibleEnvironmentManager::spawnDefaultProps()
    {
        m_Props.clear();
        m_Props.push_back({{8.0f, 8.0f, 0.0f}, DestructibleKind::RustedSteamCar, 250.0f, false});
        m_Props.push_back({{12.0f, 6.0f, 0.0f}, DestructibleKind::FragileGlassPanel, 20.0f, false});
        m_Props.push_back({{6.0f, 12.0f, 0.0f}, DestructibleKind::TrashDebrisPile, 40.0f, false});
    }

    void DestructibleEnvironmentManager::applyBlastDisruption(GameState &gs, const Vector3D &blastOrigin, float blastRadius, float blastDamage)
    {
        float radSq = blastRadius * blastRadius;

        for (auto &prop : m_Props)
        {
            if (prop.isDestroyed)
                continue;

            float dx = prop.position.x - blastOrigin.x;
            float dy = prop.position.y - blastOrigin.y;
            float distSq = dx * dx + dy * dy;

            if (distSq <= radSq)
            {
                prop.health -= blastDamage;
                if (prop.kind == DestructibleKind::RustedSteamCar)
                {
                    // Ржавый транспорт физически сдвигается взрывной ударной волной Скорча
                    prop.position.x += (dx * 0.4f);
                    prop.position.y += (dy * 0.4f);
                    std::cout << "[PHYSICS] Взрыв миномёта сдвигает ржавый остов машины!" << std::endl;
                }

                if (prop.health <= 0.0f)
                {
                    prop.isDestroyed = true;
                    if (prop.kind == DestructibleKind::FragileGlassPanel)
                    {
                        std::cout << "[DESTRUCTION] !! ЗВОН СТЕКЛА !! Стеклянная панель разлетелась на осколки!" << std::endl;
                    }
                    gs.score += 50;
                }
            }
        }
    }

} // namespace bunker
