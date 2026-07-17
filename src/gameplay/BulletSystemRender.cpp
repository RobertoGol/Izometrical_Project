#include "gameplay/BulletSystem.hpp"
#include "gameplay/AdvancedMechanics.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void BulletSystem::render(const GameState& gs, sf::RenderWindow& window, const sf::View&) const
    {
        for (const auto& b : gs.bullets)
        {
            if (!b.isAlive)
                continue;
            sf::CircleShape dot(3.0f);
            dot.setFillColor(sf::Color(255, 240, 100));
            dot.setOrigin({3.0f, 3.0f});
            dot.setPosition(IsoMath::worldToScreen(b.current.x, b.current.y));
            window.draw(dot);
        }
    }

    bool BulletSystem::isExplosive(const Bullet& b)
    {
        return b.type == BulletType::BallisticMissile || b.type == BulletType::ArtilleryMissile;
    }

    float BulletSystem::random01()
    {
        return static_cast<float>(std::rand() % 10000) / 10000.0f;
    }

    float BulletSystem::randomSpread(float amount)
    {
        if (amount <= 0.0f)
            return 0.0f;
        return (random01() - 0.5f) * amount;
    }

    Vector3D BulletSystem::normalizeDir(Vector3D dir)
    {
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 0.0001f)
            return {1.0f, 0.0f, 0.0f};
        return {dir.x / len, dir.y / len, 0.0f};
    }


} // namespace bunker
