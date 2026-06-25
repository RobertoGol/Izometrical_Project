#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "IsoMath.hpp"
#include "Constants.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace bunker {

class BulletSystem {
public:
    BulletSystem() = default;

    // ═══════════════════════════════════════════════
    // Обновление всех снарядов за кадр
    // ═══════════════════════════════════════════════
    void update(GameState& gs, float dt) {
        for (size_t i = 0; i < gs.bullets.size();) {
            Bullet& b = gs.bullets[i];

            // ── 1. Движение снаряда ──
            b.current.x += b.direction.x * b.speed * dt;
            b.current.y += b.direction.y * b.speed * dt;

            // ── 2. Проверка дистанции полёта ──
            float tdx = b.current.x - b.start.x;
            float tdy = b.current.y - b.start.y;
            if ((tdx * tdx + tdy * tdy) >= (b.maxDistance * b.maxDistance)) {
                b.isAlive = false;
            }

            // ── 3. Столкновение со стенами ──
            int tx = static_cast<int>(b.current.x);
            int ty = static_cast<int>(b.current.y);

            if (tx < 0 || tx >= Config::MAP_WIDTH || ty < 0 || ty >= Config::MAP_HEIGHT) {
                b.isAlive = false;
            } else if (gs.sectorMap[tx][ty] == 1) {
                b.isAlive = false;

                // Разрушение стен
                if (gs.wallDurability[tx][ty] > 0) {
                    int dmg = (b.type == BulletType::BallisticMissile) ? 45 : 10;
                    gs.wallDurability[tx][ty] -= dmg;
                    if (gs.wallDurability[tx][ty] <= 0) {
                        gs.sectorMap[tx][ty] = 0;  // Стена пробита!
                    }
                }
            }

            // ── 4. Попадание во врагов ──
            if (b.isAlive) {
                for (auto& e : gs.enemies) {
                    if (!e.isAlive) continue;

                    float edx = e.position.x - b.current.x;
                    float edy = e.position.y - b.current.y;
                    float distSq = edx * edx + edy * edy;
                    float hitRadiusSq = e.radius * e.radius;

                    if (distSq <= hitRadiusSq) {
                        b.isAlive = false;

                        if (b.type == BulletType::BallisticMissile ||
                            b.type == BulletType::ArtilleryMissile) {
                            // ── Сплеш-урон ракет ──
                            processSplashDamage(gs, b);
                        } else {
                            // ── Точечный урон ──
                            float dmg = (b.type == BulletType::Pellet) ? 14.0f : 25.0f;
                            e.health -= dmg;
                            if (e.health <= 0.0f) {
                                e.isAlive = false;
                                gs.score += 150;
                            }
                        }
                        break;
                    }
                }
            }

            // ── 5. Удаление мёртвых пуль (Swap-and-Pop) ──
            if (!b.isAlive) {
                if (i != gs.bullets.size() - 1) {
                    gs.bullets[i] = std::move(gs.bullets.back());
                }
                gs.bullets.pop_back();
            } else {
                ++i;
            }
        }
    }

    // ═══════════════════════════════════════════════
    // Стрельба из оружия Пилота (Scout)
    // ═══════════════════════════════════════════════
    void fireScoutWeapon(GameState& gs, bool isAiming) {
        if (gs.fireCooldown > 0.0f) return;

        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f) return;

        Vector3D normDir = { dx / dLen, dy / dLen, 0.0f };

        if (isAiming) {
            // ── Дробовик (5 лучей) ──
            for (int i = 0; i < Config::SHOTGUN_PELLETS; ++i) {
                Bullet p;
                p.start   = gs.playerPos;
                p.current = gs.playerPos;
                p.type    = BulletType::Pellet;
                float spread = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * Config::PELLET_SPREAD;
                p.direction = { normDir.x + spread, normDir.y + spread, 0.0f };
                gs.bullets.push_back(p);
            }
            gs.fireCooldown = Config::SHOTGUN_COOLDOWN;
        } else {
            // ── Стандартный карабин ──
            Bullet b;
            b.start     = gs.playerPos;
            b.current   = gs.playerPos;
            b.direction = normDir;
            b.type      = BulletType::Standard;
            gs.bullets.push_back(b);
            gs.fireCooldown = Config::CARBINE_COOLDOWN;
        }
    }

    // ═══════════════════════════════════════════════
    // Стрельба из орудий Титана (Titan)
    // ═══════════════════════════════════════════════
    void fireTitanWeapon(GameState& gs) {
        if (gs.fireCooldown > 0.0f) return;

        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f) return;

        Vector3D normDir = { dx / dLen, dy / dLen, 0.0f };

        // Если турель повреждена — люфт прицела
        if (gs.titan.systems.turretStatus < 50.0f) {
            float broken = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.25f;
            normDir.x += broken;
            normDir.y += broken;
        }

        if (gs.titan.currentWeapon == TankWeaponMode::Cannon) {
            Bullet b;
            b.start     = gs.playerPos;
            b.current   = gs.playerPos;
            b.direction = normDir;
            b.type      = BulletType::Standard;
            b.speed     = 25.0f;
            gs.bullets.push_back(b);
            gs.fireCooldown = Config::TITAN_FIRE_RATE;
        } else {
            Bullet b;
            b.start     = gs.playerPos;
            b.current   = gs.playerPos;
            b.direction = normDir;
            b.type      = BulletType::Standard;
            b.speed     = 34.0f;
            gs.bullets.push_back(b);
            gs.fireCooldown = (gs.titan.systems.turretStatus < 50.0f) ? 0.18f : 0.07f;
        }
    }

    // ═══════════════════════════════════════════════
    // Ракетный залп Титана (СКМ)
    // ═══════════════════════════════════════════════
    void fireTitanMissiles(GameState& gs) {
        if (gs.fireCooldown > 0.0f || !gs.titan.hasMissileModule) return;

        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f) return;

        Vector3D normDir = { dx / dLen, dy / dLen, 0.0f };

        if (gs.titan.missileMode == MissileStrikeMode::Ballistic) {
            // Баллистический залп: веер из 7 ракет
            for (int i = -3; i <= 3; ++i) {
                Bullet m;
                m.start     = gs.playerPos;
                m.current   = gs.playerPos;
                m.type      = BulletType::BallisticMissile;
                m.speed     = Config::BALLISTIC_MISSILE_SPEED;
                m.splashRadius = Config::BALLISTIC_SPLASH_RADIUS;
                Vector3D sideVec = { -normDir.y, normDir.x, 0.0f };
                m.start.x += sideVec.x * (i * 0.4f);
                m.start.y += sideVec.y * (i * 0.4f);
                m.direction = normDir;
                gs.bullets.push_back(m);
            }
        } else {
            // Артиллерийский залп: 8 минометных снарядов по площади
            for (int i = 0; i < 8; ++i) {
                Bullet m;
                m.start     = gs.playerPos;
                m.current   = gs.playerPos;
                m.type      = BulletType::ArtilleryMissile;
                m.speed     = Config::ARTILLERY_MISSILE_SPEED;
                m.splashRadius = Config::ARTILLERY_SPLASH_RADIUS;
                m.targetPos = gs.mouseWorldPos;
                m.targetPos.x += (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 2.0f;
                m.targetPos.y += (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 2.0f;
                gs.bullets.push_back(m);
            }
        }

        // Переключаем режим ракет
        gs.titan.missileMode = (gs.titan.missileMode == MissileStrikeMode::Ballistic)
            ? MissileStrikeMode::Artillery
            : MissileStrikeMode::Ballistic;

        gs.fireCooldown = Config::MISSILE_COOLDOWN;
    }

private:
    // ── Обработка сплеш-урона от ракеты ──
    void processSplashDamage(GameState& gs, const Bullet& b) {
        float splashRadiusSq = b.splashRadius * b.splashRadius;

        for (auto& e : gs.enemies) {
            if (!e.isAlive) continue;

            float sdx = e.position.x - b.current.x;
            float sdy = e.position.y - b.current.y;

            if ((sdx * sdx + sdy * sdy) <= splashRadiusSq) {
                e.health -= 85.0f;
                if (e.health <= 0.0f) {
                    e.isAlive = false;
                    gs.score += 150;
                }
            }
        }
    }
};

}  // namespace bunker
