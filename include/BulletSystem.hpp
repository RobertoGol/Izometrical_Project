#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "IsoMath.hpp"
#include "Constants.hpp"
#include "AdvancedMechanics.hpp"

#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <cstddef>

namespace bunker {

// ══════════════════════════════════════════════════════════════════════
// BulletSystem
//
// Обновлённый файл: баллистика теперь умеет работать не только со стенами и
// врагами, но и с перенесёнными advanced-механиками:
//   - Reactive breakables: стекло, ящики, бочки, консоли, растительность
//   - Shock waves от ракет/артиллерии/взрывных бочек
//   - Skill damage multipliers: FootKill / TankAction
//   - Weather spread penalty: AshStorm / EtherFog влияют на разброс
//   - Tank thermal load вызывается из main.cpp, но ракеты здесь дают heat-safe
//     explosion callbacks через AdvancedMechanics
//
// ВАЖНО: старые сигнатуры оставлены, чтобы проект не ломался. Новые сигнатуры
// принимают AdvancedMechanics* и используются из обновлённого main.cpp.
// ══════════════════════════════════════════════════════════════════════

class BulletSystem {
public:
    BulletSystem() = default;

    // Старый совместимый update.
    void update(GameState& gs, float dt) {
        update(gs, dt, nullptr);
    }

    // Новый update с advanced-интеграцией.
    void update(GameState& gs, float dt, AdvancedMechanics* adv) {
        for (std::size_t i = 0; i < gs.bullets.size();) {
            Bullet& b = gs.bullets[i];
            const Vector3D previousPos = b.current;
            bool explodedThisFrame = false;

            // ── 1. Движение снаряда ──
            if (b.type == BulletType::ArtilleryMissile) {
                // Артиллерия летит к targetPos, а не просто по direction.
                Vector3D toTarget = b.targetPos - b.current;
                Vector3D dir = toTarget.normalized();
                b.direction = dir;
                b.current.x += dir.x * b.speed * dt;
                b.current.y += dir.y * b.speed * dt;

                if (toTarget.lengthSq() <= 0.22f * 0.22f) {
                    b.isAlive = false;
                    explodedThisFrame = true;
                    processSplashDamage(gs, b, adv);
                }
            } else {
                b.current.x += b.direction.x * b.speed * dt;
                b.current.y += b.direction.y * b.speed * dt;
            }

            // ── 2. Проверка дистанции полёта ──
            float tdx = b.current.x - b.start.x;
            float tdy = b.current.y - b.start.y;
            if ((tdx * tdx + tdy * tdy) >= (b.maxDistance * b.maxDistance)) {
                if (isExplosive(b) && !explodedThisFrame) {
                    explodedThisFrame = true;
                    processSplashDamage(gs, b, adv);
                }
                b.isAlive = false;
            }

            // ── 3. Столкновение со стенами ──
            int tx = static_cast<int>(b.current.x);
            int ty = static_cast<int>(b.current.y);

            if (b.isAlive) {
                if (tx < 0 || tx >= Config::MAP_WIDTH || ty < 0 || ty >= Config::MAP_HEIGHT) {
                    if (isExplosive(b) && !explodedThisFrame) {
                        b.current = previousPos;
                        explodedThisFrame = true;
                        processSplashDamage(gs, b, adv);
                    }
                    b.isAlive = false;
                } else if (gs.sectorMap[tx][ty] == 1) {
                    if (isExplosive(b)) {
                        explodedThisFrame = true;
                        processSplashDamage(gs, b, adv);
                    }

                    b.isAlive = false;

                    // Разрушение стен: ракеты и артиллерия сильнее пробивают.
                    if (gs.wallDurability[tx][ty] > 0) {
                        int dmg = 10;
                        if (b.type == BulletType::BallisticMissile) dmg = 55;
                        if (b.type == BulletType::ArtilleryMissile) dmg = 70;
                        if (b.type == BulletType::Pellet) dmg = 6;

                        gs.wallDurability[tx][ty] -= dmg;
                        if (gs.wallDurability[tx][ty] <= 0) {
                            gs.sectorMap[tx][ty] = 0;  // Стена пробита!
                        }
                    }

                    if (adv && !isExplosive(b)) {
                        adv->reactive.damageAt(gs, b.current, 0.45f, 12.0f, 1.0f);
                    }
                }
            }

            // ── 4. Попадание в reactive breakables ──
            // У breakables нет отдельной hit-query, поэтому используем damageAt:
            // маленький радиус для обычных пуль, большой — для взрывчатки.
            if (b.isAlive && adv) {
                float hitRadius = (b.type == BulletType::Pellet) ? 0.22f : 0.28f;
                float hitDamage = (b.type == BulletType::Pellet) ? 10.0f : 18.0f;
                if (isExplosive(b)) { hitRadius = b.splashRadius; hitDamage = 70.0f; }

                // Лёгкий ray-step: бьём в текущей и предыдущей точке, чтобы быстрые
                // пули не пролетали сквозь ящик между кадрами.
                adv->reactive.damageAt(gs, b.current, hitRadius, hitDamage, isExplosive(b) ? 4.5f : 0.75f);
                if (!isExplosive(b)) {
                    adv->reactive.damageAt(gs, previousPos, hitRadius * 0.75f, hitDamage * 0.35f, 0.35f);
                }
            }

            // ── 5. Попадание во врагов ──
            if (b.isAlive) {
                for (auto& e : gs.enemies) {
                    if (!e.isAlive) continue;

                    float edx = e.position.x - b.current.x;
                    float edy = e.position.y - b.current.y;
                    float distSq = edx * edx + edy * edy;
                    float hitRadiusSq = e.radius * e.radius;

                    if (distSq <= hitRadiusSq) {
                        b.isAlive = false;

                        if (isExplosive(b)) {
                            // ── Сплеш-урон ракет ──
                            if (!explodedThisFrame) processSplashDamage(gs, b, adv);
                        } else {
                            // ── Точечный урон ──
                            float dmg = (b.type == BulletType::Pellet) ? 14.0f : 25.0f;
                            if (adv) dmg *= (gs.playerMode == UnitMode::Titan)
                                ? adv->skills.tankDamageMultiplier()
                                : adv->skills.footDamageMultiplier();

                            e.health -= dmg;
                            if (e.health <= 0.0f) {
                                e.isAlive = false;
                                gs.score += 150;
                                if (adv) {
                                    adv->skills.grantXp(gs, 60);
                                    adv->profile.registerKill(adv->playerProfile);
                                }
                            }
                        }
                        break;
                    }
                }
            }

            // ── 6. Удаление мёртвых пуль (Swap-and-Pop) ──
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

    // Старый совместимый выстрел Scout.
    void fireScoutWeapon(GameState& gs, bool isAiming) {
        fireScoutWeapon(gs, isAiming, nullptr);
    }

    // Новый выстрел Scout с weather/skill/stress spread.
    void fireScoutWeapon(GameState& gs, bool isAiming, AdvancedMechanics* adv) {
        if (gs.fireCooldown > 0.0f) return;

        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f) return;

        Vector3D normDir = { dx / dLen, dy / dLen, 0.0f };
        const float advancedSpread = adv ? (adv->weather.bulletSpreadPenalty() + adv->survival.aimPenalty()) : 0.0f;

        if (isAiming) {
            // ── Дробовик (5 лучей) ──
            for (int i = 0; i < Config::SHOTGUN_PELLETS; ++i) {
                Bullet p;
                p.start   = gs.playerPos;
                p.current = gs.playerPos;
                p.type    = BulletType::Pellet;
                float spread = randomSpread(Config::PELLET_SPREAD + advancedSpread);
                p.direction = normalizeDir({ normDir.x + spread, normDir.y - spread, 0.0f });
                gs.bullets.push_back(p);
            }
            gs.fireCooldown = Config::SHOTGUN_COOLDOWN;
        } else {
            // ── Стандартный карабин ──
            Bullet b;
            b.start     = gs.playerPos;
            b.current   = gs.playerPos;
            b.direction = normalizeDir({ normDir.x + randomSpread(advancedSpread),
                                         normDir.y + randomSpread(advancedSpread), 0.0f });
            b.type      = BulletType::Standard;
            gs.bullets.push_back(b);
            gs.fireCooldown = Config::CARBINE_COOLDOWN;
        }
    }

    // Старое совместимое орудие Титана.
    void fireTitanWeapon(GameState& gs) {
        fireTitanWeapon(gs, nullptr);
    }

    // Новое орудие Титана с weather/skill spread.
    void fireTitanWeapon(GameState& gs, AdvancedMechanics* adv) {
        if (gs.fireCooldown > 0.0f) return;

        float dx = gs.mouseWorldPos.x - gs.titan.position.x;
        float dy = gs.mouseWorldPos.y - gs.titan.position.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f) return;

        Vector3D normDir = { dx / dLen, dy / dLen, 0.0f };

        // Если турель повреждена — люфт прицела.
        if (gs.titan.systems.turretStatus < 50.0f) {
            float broken = randomSpread(0.25f);
            normDir.x += broken;
            normDir.y -= broken;
        }

        if (adv) {
            float weatherPenalty = adv->weather.bulletSpreadPenalty() * 0.55f;
            normDir.x += randomSpread(weatherPenalty);
            normDir.y += randomSpread(weatherPenalty);
        }

        normDir = normalizeDir(normDir);

        if (gs.titan.currentWeapon == TankWeaponMode::Cannon) {
            Bullet b;
            b.start     = gs.titan.position;
            b.current   = gs.titan.position;
            b.direction = normDir;
            b.type      = BulletType::Standard;
            b.speed     = 25.0f;
            b.maxDistance = Config::BULLET_MAX_RANGE * 1.35f;
            gs.bullets.push_back(b);
            gs.fireCooldown = Config::TITAN_FIRE_RATE;
        } else {
            Bullet b;
            b.start     = gs.titan.position;
            b.current   = gs.titan.position;
            b.direction = normDir;
            b.type      = BulletType::Standard;
            b.speed     = 34.0f;
            b.maxDistance = Config::BULLET_MAX_RANGE * 1.15f;
            gs.bullets.push_back(b);
            gs.fireCooldown = (gs.titan.systems.turretStatus < 50.0f) ? 0.18f : 0.07f;
        }
    }

    // Старый совместимый ракетный залп.
    void fireTitanMissiles(GameState& gs) {
        fireTitanMissiles(gs, nullptr);
    }

    // Новый ракетный залп с advanced callbacks.
    void fireTitanMissiles(GameState& gs, AdvancedMechanics* adv) {
        if (gs.fireCooldown > 0.0f || !gs.titan.hasMissileModule) return;

        float dx = gs.mouseWorldPos.x - gs.titan.position.x;
        float dy = gs.mouseWorldPos.y - gs.titan.position.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f) return;

        Vector3D normDir = { dx / dLen, dy / dLen, 0.0f };
        if (adv) {
            float weatherPenalty = adv->weather.bulletSpreadPenalty() * 0.4f;
            normDir.x += randomSpread(weatherPenalty);
            normDir.y += randomSpread(weatherPenalty);
            normDir = normalizeDir(normDir);
        }

        if (gs.titan.missileMode == MissileStrikeMode::Ballistic) {
            // Баллистический залп: веер из 7 ракет.
            for (int i = -3; i <= 3; ++i) {
                Bullet m;
                m.start     = gs.titan.position;
                m.current   = gs.titan.position;
                m.type      = BulletType::BallisticMissile;
                m.speed     = Config::BALLISTIC_MISSILE_SPEED;
                m.splashRadius = Config::BALLISTIC_SPLASH_RADIUS;
                Vector3D sideVec = { -normDir.y, normDir.x, 0.0f };
                m.start.x += sideVec.x * (i * 0.4f);
                m.start.y += sideVec.y * (i * 0.4f);
                m.current = m.start;
                m.direction = normDir;
                gs.bullets.push_back(m);
            }
        } else {
            // Артиллерийский залп: 8 минометных снарядов по площади.
            for (int i = 0; i < 8; ++i) {
                Bullet m;
                m.start     = gs.titan.position;
                m.current   = gs.titan.position;
                m.type      = BulletType::ArtilleryMissile;
                m.speed     = Config::ARTILLERY_MISSILE_SPEED;
                m.splashRadius = Config::ARTILLERY_SPLASH_RADIUS;
                m.targetPos = gs.mouseWorldPos;
                m.targetPos.x += randomSpread(2.0f);
                m.targetPos.y += randomSpread(2.0f);
                m.direction = normalizeDir(m.targetPos - m.current);
                gs.bullets.push_back(m);
            }
        }

        // Переключаем режим ракет.
        gs.titan.missileMode = (gs.titan.missileMode == MissileStrikeMode::Ballistic)
            ? MissileStrikeMode::Artillery
            : MissileStrikeMode::Ballistic;

        gs.fireCooldown = Config::MISSILE_COOLDOWN;
    }

private:
    static bool isExplosive(const Bullet& b) {
        return b.type == BulletType::BallisticMissile || b.type == BulletType::ArtilleryMissile;
    }

    static float random01() {
        return static_cast<float>(std::rand() % 10000) / 10000.0f;
    }

    static float randomSpread(float amount) {
        if (amount <= 0.0f) return 0.0f;
        return (random01() - 0.5f) * amount;
    }

    static Vector3D normalizeDir(Vector3D dir) {
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 0.0001f) return {1.0f, 0.0f, 0.0f};
        return {dir.x / len, dir.y / len, 0.0f};
    }

    // ── Обработка сплеш-урона от ракеты ──
    void processSplashDamage(GameState& gs, const Bullet& b, AdvancedMechanics* adv) {
        float splashRadius = std::max(0.1f, b.splashRadius);
        float splashRadiusSq = splashRadius * splashRadius;

        if (adv) {
            adv->onExplosion(gs, b.current, splashRadius, 85.0f);
        }

        for (auto& e : gs.enemies) {
            if (!e.isAlive) continue;

            float sdx = e.position.x - b.current.x;
            float sdy = e.position.y - b.current.y;
            float distSq = sdx * sdx + sdy * sdy;

            if (distSq <= splashRadiusSq) {
                float dist = std::sqrt(std::max(0.0001f, distSq));
                float falloff = 1.0f - std::min(1.0f, dist / splashRadius);
                float dmg = 85.0f * std::max(0.35f, falloff);
                if (adv) dmg *= adv->skills.tankDamageMultiplier();

                e.health -= dmg;
                // Ударной волной отталкиваем врага от эпицентра.
                Vector3D away = normalizeDir(e.position - b.current);
                e.position += away * (0.35f + falloff * 0.65f);

                if (e.health <= 0.0f) {
                    e.isAlive = false;
                    gs.score += 150;
                    if (adv) {
                        adv->skills.grantXp(gs, 80);
                        adv->profile.registerKill(adv->playerProfile);
                    }
                }
            }
        }
    }
};

}  // namespace bunker
