#include "ai/HostileAISystem.hpp"
#include "ai/PerceptionSystem.hpp"
#include "ai/EnemyArchetypeRegistry.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

namespace bunker
{

    int HostileAISystem::spawnHostile(GameState &gs, HostileKind kind, const Vector3D &pos)
    {
        Enemy e;
        e.position = pos;
        e.isAlive = true;

        HostileProfile p = profileFor(kind);
        e.speed = p.speed;
        e.radius = radiusFor(kind);
        e.health = healthFor(kind);

        gs.enemies.push_back(e);
        ensureStateSize(gs);

        const int index = static_cast<int>(gs.enemies.size()) - 1;
        m_State[index] = makeRuntimeState(kind);
        return index;
    }

    void HostileAISystem::assignKind(GameState &gs, std::size_t enemyIndex, HostileKind kind)
    {
        ensureStateSize(gs);
        if (enemyIndex >= gs.enemies.size())
            return;

        m_State[enemyIndex] = makeRuntimeState(kind);
        HostileProfile p = profileFor(kind);
        gs.enemies[enemyIndex].speed = p.speed;
        gs.enemies[enemyIndex].radius = radiusFor(kind);
        if (gs.enemies[enemyIndex].health <= 0.0f)
        {
            gs.enemies[enemyIndex].health = healthFor(kind);
        }
    }

    void HostileAISystem::update(GameState &gs, float dt)
    {
        ensureStateSize(gs);

        for (std::size_t i = 0; i < gs.enemies.size(); ++i)
        {
            Enemy &e = gs.enemies[i];
            HostileRuntimeState &st = m_State[i];

            if (!e.isAlive)
                continue;
            if (e.health <= 0.0f)
            {
                e.isAlive = false;
                continue;
            }

            HostileProfile p = profileFor(st.kind);
            applyMechanicalDegradation(p, st, e);

            if (st.attackTimer > 0.0f)
                st.attackTimer -= dt;
            if (st.strafeTimer > 0.0f)
                st.strafeTimer -= dt;
            if (st.strafeTimer <= 0.0f)
            {
                st.strafeTimer = 1.0f + random01() * 1.5f;
                st.strafeSign = (random01() < 0.5f) ? -1.0f : 1.0f;
            }

            Vector3D target = chooseTarget(gs, e);
            float dx = target.x - e.position.x;
            float dy = target.y - e.position.y;
            float distSq = dx * dx + dy * dy;
            float dist = std::sqrt(std::max(distSq, 0.0001f));

            bool hasLOS = PerceptionSystem::hasLineOfSight(gs, e.position, target);
            updateAwareness(st, p, target, dist, dt, gs.worldVisibilityModifier, hasLOS);

            switch (st.alert)
            {
            case HostileAlertState::Idle:
                updateIdle(e, dt);
                break;

            case HostileAlertState::Suspicious:
                moveToward(gs, e, st.lastKnownTarget, p.speed * 0.55f, dt);
                break;

            case HostileAlertState::Aggro:
                updateAggro(gs, e, st, p, target, dist, dt);
                break;

            case HostileAlertState::Searching:
                updateSearching(gs, e, st, p, dt);
                break;
            }
        }
    }

    void HostileAISystem::applyDamageToHostile(GameState &gs, std::size_t enemyIndex, float damage, float hitLocalX)
    {
        ensureStateSize(gs);
        if (enemyIndex >= gs.enemies.size())
            return;

        Enemy &e = gs.enemies[enemyIndex];
        HostileRuntimeState &st = m_State[enemyIndex];

        if (!e.isAlive)
            return;

        if (profileFor(st.kind).isMechanical)
        {
            if (hitLocalX < -0.33f)
            {
                st.mech.mobility = std::max(0.0f, st.mech.mobility - damage * 1.15f);
            }
            else if (hitLocalX > 0.33f)
            {
                st.mech.weapon = std::max(0.0f, st.mech.weapon - damage * 1.10f);
            }
            else
            {
                st.mech.sensors = std::max(0.0f, st.mech.sensors - damage * 0.90f);
            }

            DamageSystem::applyEnemyDamage(gs, e, damage * 0.65f, DamageType::Kinetic);
        }
        else
        {
            DamageSystem::applyEnemyDamage(gs, e, damage, DamageType::Kinetic);
        }

        st.awareness = std::min(profileFor(st.kind).aggroThreshold, st.awareness + 45.0f);
        st.alert = HostileAlertState::Aggro;
        st.lastKnownTarget = gs.playerPos;
    }

    HostileProfile HostileAISystem::profileFor(HostileKind kind)
    {
        return EnemyArchetypeRegistry::getProfile(kind);
    }

    float HostileAISystem::healthFor(HostileKind kind)
    {
        switch (kind)
        {
        case HostileKind::VerminRush:
            return 35.0f;
        case HostileKind::GhoulRush:
            return 85.0f;
        case HostileKind::HumanTactical:
            return 65.0f;
        case HostileKind::RobotControl:
            return 140.0f;
        }
        return Config::ENEMY_BASE_HP;
    }

    float HostileAISystem::radiusFor(HostileKind kind)
    {
        switch (kind)
        {
        case HostileKind::VerminRush:
            return 0.22f;
        case HostileKind::GhoulRush:
            return 0.34f;
        case HostileKind::HumanTactical:
            return 0.28f;
        case HostileKind::RobotControl:
            return 0.42f;
        }
        return Config::ENEMY_RADIUS;
    }

    void HostileAISystem::ensureStateSize(const GameState &gs)
    {
        if (m_State.size() == gs.enemies.size())
            return;

        std::size_t oldSize = m_State.size();
        m_State.resize(gs.enemies.size());

        for (std::size_t i = oldSize; i < m_State.size(); ++i)
        {
            HostileKind kind = HostileKind::VerminRush;
            if (i % 4 == 1)
                kind = HostileKind::GhoulRush;
            if (i % 4 == 2)
                kind = HostileKind::HumanTactical;
            if (i % 4 == 3)
                kind = HostileKind::RobotControl;
            m_State[i] = makeRuntimeState(kind);
        }
    }

    HostileRuntimeState HostileAISystem::makeRuntimeState(HostileKind kind) const
    {
        HostileRuntimeState st;
        st.kind = kind;
        st.alert = HostileAlertState::Idle;
        st.awareness = 0.0f;
        st.attackTimer = random01() * 0.4f;
        st.strafeSign = (random01() < 0.5f) ? -1.0f : 1.0f;
        st.strafeTimer = 0.5f + random01();
        return st;
    }

    void HostileAISystem::applyMechanicalDegradation(HostileProfile &p, const HostileRuntimeState &st, Enemy &e) const
    {
        if (!p.isMechanical)
            return;

        float sensorFactor = std::clamp(st.mech.sensors / 100.0f, 0.25f, 1.0f);
        float weaponFactor = std::clamp(st.mech.weapon / 100.0f, 0.15f, 1.0f);
        float moveFactor = std::clamp(st.mech.mobility / 100.0f, 0.20f, 1.0f);

        p.detectRadius *= sensorFactor;
        p.loseRadius *= std::max(sensorFactor, 0.45f);
        p.damage *= weaponFactor;
        p.speed *= moveFactor;
        e.speed = p.speed;
    }

    void HostileAISystem::updateAwareness(HostileRuntimeState &st, const HostileProfile &p,
                                          const Vector3D &target, float dist, float dt, float visModifier, bool hasLOS) const
    {
        float effRadius = p.detectRadius * std::clamp(visModifier, 0.2f, 1.0f);
        bool canSense = (dist <= effRadius) && hasLOS;
        bool tooFar = dist > p.loseRadius;

        if (canSense)
        {
            st.awareness = std::min(p.aggroThreshold, st.awareness + p.awarenessGain * dt);
            st.lastKnownTarget = target;

            if (st.awareness >= p.aggroThreshold)
            {
                st.alert = HostileAlertState::Aggro;
                st.searchTimer = 2.5f;
            }
            else if (st.alert == HostileAlertState::Idle)
            {
                st.alert = HostileAlertState::Suspicious;
            }
        }
        else
        {
            st.awareness = std::max(0.0f, st.awareness - p.awarenessDecay * dt);

            if (st.alert == HostileAlertState::Aggro && tooFar)
            {
                st.alert = HostileAlertState::Searching;
                st.searchTimer = 3.0f;
            }
            else if (st.awareness <= 0.0f)
            {
                st.alert = HostileAlertState::Idle;
            }
        }
    }

    void HostileAISystem::updateIdle(Enemy &e, float dt) const
    {
        float wobbleX = (random01() - 0.5f) * 0.15f * dt;
        float wobbleY = (random01() - 0.5f) * 0.15f * dt;
        e.position.x += wobbleX;
        e.position.y += wobbleY;
    }

    void HostileAISystem::updateSearching(GameState &gs, Enemy &e, HostileRuntimeState &st,
                                          const HostileProfile &p, float dt)
    {
        st.searchTimer -= dt;
        moveToward(gs, e, st.lastKnownTarget, p.speed * 0.70f, dt);

        if (st.searchTimer <= 0.0f)
        {
            st.alert = HostileAlertState::Suspicious;
        }
    }

    void HostileAISystem::updateAggro(GameState &gs, Enemy &e, HostileRuntimeState &st,
                                      const HostileProfile &p, const Vector3D &target,
                                      float dist, float dt)
    {
        if (p.usesRangedAttack)
        {
            updateRanged(gs, e, st, p, target, dist, dt);
        }
        else
        {
            updateMelee(gs, e, st, p, target, dist, dt);
        }
    }

    void HostileAISystem::updateMelee(GameState &gs, Enemy &e, HostileRuntimeState &st,
                                      const HostileProfile &p, const Vector3D &target,
                                      float dist, float dt)
    {
        if (dist > p.attackRadius)
        {
            float rush = (st.kind == HostileKind::GhoulRush && dist < 2.3f) ? 1.35f : 1.0f;
            moveToward(gs, e, target, p.speed * rush, dt);
            return;
        }

        performAttack(gs, st, p);
    }

    void HostileAISystem::updateRanged(GameState &gs, Enemy &e, HostileRuntimeState &st,
                                       const HostileProfile &p, const Vector3D &target,
                                       float dist, float dt)
    {
        if (dist > p.attackRadius)
        {
            moveToward(gs, e, target, p.speed, dt);
        }
        else if (dist < p.preferredRange * 0.65f)
        {
            moveAway(gs, e, target, p.speed * 0.85f, dt);
        }
        else
        {
            strafeAround(gs, e, target, p.speed * 0.55f, st.strafeSign, dt);
        }

        if (dist <= p.attackRadius)
        {
            performAttack(gs, st, p);
        }
    }

    void HostileAISystem::performAttack(GameState &gs, HostileRuntimeState &st, const HostileProfile &p)
    {
        if (st.attackTimer > 0.0f)
            return;

        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted)
        {
            DamageSystem::applyTitanDamage(gs, p.damage, DamageType::Kinetic);
            if (p.isMechanical)
            {
                gs.titan.systems.sensorLink = std::max(0.0f, gs.titan.systems.sensorLink - p.damage * 0.25f);
            }
        }
        else
        {
            DamageSystem::applyPlayerDamage(gs, p.damage, DamageType::Kinetic);
        }

        st.attackTimer = p.attackCooldown;
    }

    Vector3D HostileAISystem::chooseTarget(const GameState &gs, const Enemy &e) const
    {
        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted)
        {
            return gs.titan.position;
        }

        float pdx = gs.playerPos.x - e.position.x;
        float pdy = gs.playerPos.y - e.position.y;
        float tdx = gs.titan.position.x - e.position.x;
        float tdy = gs.titan.position.y - e.position.y;
        float playerDistSq = pdx * pdx + pdy * pdy;
        float titanDistSq = tdx * tdx + tdy * tdy;

        if (gs.titan.health > 0.0f && titanDistSq + 4.0f < playerDistSq)
        {
            return gs.titan.position;
        }

        return gs.playerPos;
    }

    void HostileAISystem::moveToward(GameState &gs, Enemy &e, const Vector3D &target, float speed, float dt) const
    {
        float dx = target.x - e.position.x;
        float dy = target.y - e.position.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;
        tryMove(gs, e, dx / len, dy / len, speed, dt);
    }

    void HostileAISystem::moveAway(GameState &gs, Enemy &e, const Vector3D &target, float speed, float dt) const
    {
        float dx = e.position.x - target.x;
        float dy = e.position.y - target.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;
        tryMove(gs, e, dx / len, dy / len, speed, dt);
    }

    void HostileAISystem::strafeAround(GameState &gs, Enemy &e, const Vector3D &target,
                                       float speed, float sign, float dt) const
    {
        float dx = target.x - e.position.x;
        float dy = target.y - e.position.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;

        float nx = dx / len;
        float ny = dy / len;

        tryMove(gs, e, -ny * sign, nx * sign, speed, dt);
    }

    void HostileAISystem::tryMove(GameState &gs, Enemy &e, float dirX, float dirY, float speed, float dt) const
    {
        float nx = e.position.x + dirX * speed * dt;
        float ny = e.position.y + dirY * speed * dt;

        if (!Collisions::checkWorldCollision(gs, nx, ny, e.radius))
        {
            e.position.x = nx;
            e.position.y = ny;
            return;
        }

        if (!Collisions::checkWorldCollision(gs, nx, e.position.y, e.radius))
        {
            e.position.x = nx;
        }
        if (!Collisions::checkWorldCollision(gs, e.position.x, ny, e.radius))
        {
            e.position.y = ny;
        }
    }

    float HostileAISystem::random01()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

} // namespace bunker