#include "gameplay/BulletSystem.hpp"
#include "gameplay/AdvancedMechanics.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void BulletSystem::fireScoutWeapon(GameState& gs, bool isAiming)
    {
        fireScoutWeapon(gs, isAiming, nullptr);
    }

    void BulletSystem::fireScoutWeapon(GameState& gs, bool isAiming, AdvancedMechanics* adv)
    {
        if (gs.fireCooldown > 0.0f)
            return;

        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f)
            return;

        Vector3D normDir = {dx / dLen, dy / dLen, 0.0f};
        const float advancedSpread = adv ? (adv->weather.bulletSpreadPenalty() + adv->survival.aimPenalty()) : 0.0f;

        if (isAiming)
        {
            for (int i = 0; i < Config::SHOTGUN_PELLETS; ++i)
            {
                Bullet p;
                p.start = gs.playerPos;
                p.current = gs.playerPos;
                p.type = BulletType::Pellet;
                float spread = randomSpread(Config::PELLET_SPREAD + advancedSpread);
                p.direction = normalizeDir({normDir.x + spread, normDir.y - spread, 0.0f});
                gs.bullets.push_back(p);
            }
            gs.fireCooldown = Config::SHOTGUN_COOLDOWN;
        }
        else
        {
            Bullet b;
            b.start = gs.playerPos;
            b.current = gs.playerPos;
            b.direction = normalizeDir(
                {normDir.x + randomSpread(advancedSpread), normDir.y + randomSpread(advancedSpread), 0.0f});
            b.type = BulletType::Standard;
            gs.bullets.push_back(b);
            gs.fireCooldown = Config::CARBINE_COOLDOWN;
        }
    }

    void BulletSystem::fireTitanWeapon(GameState& gs)
    {
        fireTitanWeapon(gs, nullptr);
    }

    void BulletSystem::fireTitanWeapon(GameState& gs, AdvancedMechanics* adv)
    {
        if (gs.fireCooldown > 0.0f)
            return;

        float dx = gs.mouseWorldPos.x - gs.titan.position.x;
        float dy = gs.mouseWorldPos.y - gs.titan.position.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f)
            return;

        Vector3D normDir = {dx / dLen, dy / dLen, 0.0f};

        if (gs.titan.systems.turretStatus < 50.0f)
        {
            float broken = randomSpread(0.25f);
            normDir.x += broken;
            normDir.y -= broken;
        }

        if (adv)
        {
            float weatherPenalty = adv->weather.bulletSpreadPenalty() * 0.55f;
            normDir.x += randomSpread(weatherPenalty);
            normDir.y += randomSpread(weatherPenalty);
        }

        normDir = normalizeDir(normDir);

        if (gs.titan.currentWeapon == TankWeaponMode::Cannon)
        {
            Bullet b;
            b.start = gs.titan.position;
            b.current = gs.titan.position;
            b.direction = normDir;
            b.type = BulletType::Standard;
            b.speed = 25.0f;
            b.maxDistance = Config::BULLET_MAX_RANGE * 1.35f;
            gs.bullets.push_back(b);
            gs.fireCooldown = Config::TITAN_FIRE_RATE;
        }
        else
        {
            Bullet b;
            b.start = gs.titan.position;
            b.current = gs.titan.position;
            b.direction = normDir;
            b.type = BulletType::Standard;
            b.speed = 34.0f;
            b.maxDistance = Config::BULLET_MAX_RANGE * 1.15f;
            gs.bullets.push_back(b);
            gs.fireCooldown = (gs.titan.systems.turretStatus < 50.0f) ? 0.18f : 0.07f;
        }
    }

    void BulletSystem::fireTitanMissiles(GameState& gs)
    {
        fireTitanMissiles(gs, nullptr);
    }

    void BulletSystem::fireTitanMissiles(GameState& gs, AdvancedMechanics* adv)
    {
        if (gs.fireCooldown > 0.0f || !gs.titan.hasMissileModule)
            return;

        float dx = gs.mouseWorldPos.x - gs.titan.position.x;
        float dy = gs.mouseWorldPos.y - gs.titan.position.y;
        float dLen = std::sqrt(dx * dx + dy * dy);
        if (dLen < 0.05f)
            return;

        Vector3D normDir = {dx / dLen, dy / dLen, 0.0f};
        if (adv)
        {
            float weatherPenalty = adv->weather.bulletSpreadPenalty() * 0.4f;
            normDir.x += randomSpread(weatherPenalty);
            normDir.y += randomSpread(weatherPenalty);
            normDir = normalizeDir(normDir);
        }

        if (gs.titan.missileMode == MissileStrikeMode::Ballistic)
        {
            for (int i = -3; i <= 3; ++i)
            {
                Bullet m;
                m.start = gs.titan.position;
                m.current = gs.titan.position;
                m.type = BulletType::BallisticMissile;
                m.speed = Config::BALLISTIC_MISSILE_SPEED;
                m.splashRadius = Config::BALLISTIC_SPLASH_RADIUS;
                Vector3D sideVec = {-normDir.y, normDir.x, 0.0f};
                m.start.x += sideVec.x * (i * 0.4f);
                m.start.y += sideVec.y * (i * 0.4f);
                m.current = m.start;
                m.direction = normDir;
                gs.bullets.push_back(m);
            }
        }
        else
        {
            for (int i = 0; i < 8; ++i)
            {
                Bullet m;
                m.start = gs.titan.position;
                m.current = gs.titan.position;
                m.type = BulletType::ArtilleryMissile;
                m.speed = Config::ARTILLERY_MISSILE_SPEED;
                m.splashRadius = Config::ARTILLERY_SPLASH_RADIUS;
                m.targetPos = gs.mouseWorldPos;
                m.targetPos.x += randomSpread(2.0f);
                m.targetPos.y += randomSpread(2.0f);
                m.direction = normalizeDir(m.targetPos - m.current);
                gs.bullets.push_back(m);
            }
        }

        gs.titan.missileMode = (gs.titan.missileMode == MissileStrikeMode::Ballistic) ? MissileStrikeMode::Artillery
                                                                                      : MissileStrikeMode::Ballistic;

        gs.fireCooldown = Config::MISSILE_COOLDOWN;
    }


} // namespace bunker
