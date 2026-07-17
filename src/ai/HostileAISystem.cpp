#include "ai/HostileAISystem.hpp"
#include "ai/EnemyArchetypeRegistry.hpp"
#include "ai/PerceptionSystem.hpp"
#include "engine/Log.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    int HostileAISystem::spawnHostile(GameState& gs, HostileKind kind, const Vector3D& pos)
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

    void HostileAISystem::assignKind(GameState& gs, std::size_t enemyIndex, HostileKind kind)
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

    void HostileAISystem::update(GameState& gs, float dt)
    {
        ensureStateSize(gs);

        for (std::size_t i = 0; i < gs.enemies.size(); ++i)
        {
            Enemy& e = gs.enemies[i];
            HostileRuntimeState& st = m_State[i];

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
            default:
                break;
            }
        }
    }

    void HostileAISystem::applyDamageToHostile(GameState& gs, std::size_t enemyIndex, float damage, float hitLocalX)
    {
        ensureStateSize(gs);
        if (enemyIndex >= gs.enemies.size())
            return;

        Enemy& e = gs.enemies[enemyIndex];
        HostileRuntimeState& st = m_State[enemyIndex];

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


} // namespace bunker
