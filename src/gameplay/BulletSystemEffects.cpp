#include "gameplay/BulletSystem.hpp"
#include "gameplay/AdvancedMechanics.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void BulletSystem::processSplashDamage(GameState& gs, const Bullet& b, AdvancedMechanics* adv)
    {
        float splashRadius = std::max(0.1f, b.splashRadius);
        float splashRadiusSq = splashRadius * splashRadius;

        if (adv)
        {
            adv->onExplosion(gs, b.current, splashRadius, 85.0f);
        }

        for (auto& e : gs.enemies)
        {
            if (!e.isAlive)
                continue;

            float sdx = e.position.x - b.current.x;
            float sdy = e.position.y - b.current.y;
            float distSq = sdx * sdx + sdy * sdy;

            if (distSq <= splashRadiusSq)
            {
                float dist = std::sqrt(std::max(0.0001f, distSq));
                float falloff = 1.0f - std::min(1.0f, dist / splashRadius);
                float dmg = 85.0f * std::max(0.35f, falloff);
                if (adv)
                    dmg *= adv->skills.tankDamageMultiplier();

                bool killed = DamageSystem::applyEnemyDamage(gs, e, dmg, DamageType::Explosive);
                Vector3D away = normalizeDir(e.position - b.current);
                e.position += away * (0.35f + falloff * 0.65f);

                if (killed)
                {
                    if (adv)
                    {
                        adv->skills.grantXp(gs, 80);
                        adv->profile.registerKill(adv->playerProfile);
                    }
                }
            }
        }
    }

    void BulletSystem::fireDebugGunChainLightning(GameState& gs)
    {
        if (gs.enemies.empty())
            return;
        Vector3D curr = gs.playerPos;
        for (auto& e : gs.enemies)
        {
            if (!e.isAlive)
                continue;
            float dx = curr.x - e.position.x;
            float dy = curr.y - e.position.y;
            if ((dx * dx + dy * dy) < 100.0f)
            {
                Bullet b;
                b.start = curr;
                b.current = e.position;
                b.type = BulletType::Standard;
                b.speed = 100.0f;
                gs.bullets.push_back(b);
                DamageSystem::applyEnemyDamage(gs, e, 500.0f, DamageType::Electric);
                curr = e.position;
            }
        }
        gs.fireCooldown = 0.25f;
    }


} // namespace bunker
