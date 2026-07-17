#include "gameplay/BulletSystem.hpp"
#include "gameplay/AdvancedMechanics.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void BulletSystem::update(GameState& gs, float dt)
    {
        update(gs, dt, nullptr);
    }

    void BulletSystem::update(GameState& gs, float dt, AdvancedMechanics* adv)
    {
        for (std::size_t i = 0; i < gs.bullets.size();)
        {
            Bullet& b = gs.bullets[i];
            bool explodedThisFrame = false;

            int numSubSteps = 1;
            float stepDist = b.speed * dt;
            if (stepDist > 0.2f)
            {
                numSubSteps = static_cast<int>(std::ceil(stepDist / 0.2f));
            }
            float sub_dt = dt / numSubSteps;

            for (int step = 0; step < numSubSteps && b.isAlive; ++step)
            {
                const Vector3D previousPos = b.current;

                if (b.type == BulletType::ArtilleryMissile)
                {
                    Vector3D toTarget = b.targetPos - b.current;
                    Vector3D dir = toTarget.normalized();
                    b.direction = dir;
                    b.current.x += dir.x * b.speed * sub_dt;
                    b.current.y += dir.y * b.speed * sub_dt;

                    if (toTarget.lengthSq() <= 0.22f * 0.22f)
                    {
                        b.isAlive = false;
                        explodedThisFrame = true;
                        processSplashDamage(gs, b, adv);
                        break;
                    }
                }
                else
                {
                    b.current.x += b.direction.x * b.speed * sub_dt;
                    b.current.y += b.direction.y * b.speed * sub_dt;
                }

                float tdx = b.current.x - b.start.x;
                float tdy = b.current.y - b.start.y;
                if ((tdx * tdx + tdy * tdy) >= (b.maxDistance * b.maxDistance))
                {
                    if (isExplosive(b) && !explodedThisFrame)
                    {
                        explodedThisFrame = true;
                        processSplashDamage(gs, b, adv);
                    }
                    b.isAlive = false;
                    break;
                }

                int tx = static_cast<int>(b.current.x);
                int ty = static_cast<int>(b.current.y);

                if (tx < 0 || tx >= Config::MAP_WIDTH || ty < 0 || ty >= Config::MAP_HEIGHT)
                {
                    if (isExplosive(b) && !explodedThisFrame)
                    {
                        b.current = previousPos;
                        explodedThisFrame = true;
                        processSplashDamage(gs, b, adv);
                    }
                    b.isAlive = false;
                    break;
                }
                else if (gs.sectorMap[tx][ty] == 1)
                {
                    if (isExplosive(b))
                    {
                        explodedThisFrame = true;
                        processSplashDamage(gs, b, adv);
                    }

                    b.isAlive = false;

                    if (gs.wallDurability[tx][ty] > 0)
                    {
                        int dmg = 10;
                        if (b.type == BulletType::BallisticMissile)
                            dmg = 55;
                        if (b.type == BulletType::ArtilleryMissile)
                            dmg = 70;
                        if (b.type == BulletType::Pellet)
                            dmg = 6;

                        gs.wallDurability[tx][ty] -= dmg;
                        if (gs.wallDurability[tx][ty] <= 0)
                        {
                            gs.sectorMap[tx][ty] = 0;
                        }
                    }

                    if (adv && !isExplosive(b))
                    {
                        adv->reactive.damageAt(gs, b.current, 0.45f, 12.0f, 1.0f);
                    }
                    break;
                }

                if (adv)
                {
                    float hitRadius = (b.type == BulletType::Pellet) ? 0.22f : 0.28f;
                    float hitDamage = (b.type == BulletType::Pellet) ? 10.0f : 18.0f;
                    if (isExplosive(b))
                    {
                        hitRadius = b.splashRadius;
                        hitDamage = 70.0f;
                    }

                    adv->reactive.damageAt(gs, b.current, hitRadius, hitDamage, isExplosive(b) ? 4.5f : 0.75f);
                    if (!isExplosive(b))
                    {
                        adv->reactive.damageAt(gs, previousPos, hitRadius * 0.75f, hitDamage * 0.35f, 0.35f);
                    }
                }

                for (auto& e : gs.enemies)
                {
                    if (!e.isAlive)
                        continue;

                    float edx = e.position.x - b.current.x;
                    float edy = e.position.y - b.current.y;
                    float distSq = edx * edx + edy * edy;
                    float hitRadiusSq = e.radius * e.radius;

                    if (distSq > hitRadiusSq)
                        continue;

                    b.isAlive = false;

                    if (isExplosive(b))
                    {
                        if (!explodedThisFrame)
                            processSplashDamage(gs, b, adv);
                        break;
                    }

                    float dmg = (b.type == BulletType::Pellet) ? 14.0f : 25.0f;
                    if (adv)
                        dmg *= (gs.playerMode == UnitMode::Titan) ? adv->skills.tankDamageMultiplier()
                                                                  : adv->skills.footDamageMultiplier();

                    if (DamageSystem::applyEnemyDamage(gs, e, dmg, DamageType::Kinetic) && adv)
                    {
                        adv->skills.grantXp(gs, 60);
                        adv->profile.registerKill(adv->playerProfile);
                    }
                    break;
                }
            }

            if (!b.isAlive)
            {
                if (i != gs.bullets.size() - 1)
                {
                    gs.bullets[i] = std::move(gs.bullets.back());
                }
                gs.bullets.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }


} // namespace bunker
