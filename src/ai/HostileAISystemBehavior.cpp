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

    void HostileAISystem::updateAwareness(HostileRuntimeState& st, const HostileProfile& p, const Vector3D& target,
                                          float dist, float dt, float visModifier, bool hasLOS) const
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
                return;
            }

            if (st.alert == HostileAlertState::Idle)
            {
                st.alert = HostileAlertState::Suspicious;
            }
            return;
        }

        st.awareness = std::max(0.0f, st.awareness - p.awarenessDecay * dt);

        if (st.alert == HostileAlertState::Aggro && tooFar)
        {
            st.alert = HostileAlertState::Searching;
            st.searchTimer = 3.0f;
            return;
        }

        if (st.awareness <= 0.0f)
        {
            st.alert = HostileAlertState::Idle;
        }
    }

    void HostileAISystem::updateIdle(Enemy& e, float dt) const
    {
        float wobbleX = (random01() - 0.5f) * 0.15f * dt;
        float wobbleY = (random01() - 0.5f) * 0.15f * dt;
        e.position.x += wobbleX;
        e.position.y += wobbleY;
    }

    void HostileAISystem::updateSearching(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p,
                                          float dt)
    {
        st.searchTimer -= dt;
        moveToward(gs, e, st.lastKnownTarget, p.speed * 0.70f, dt);

        if (st.searchTimer <= 0.0f)
        {
            st.alert = HostileAlertState::Suspicious;
        }
    }

    void HostileAISystem::updateAggro(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p,
                                      const Vector3D& target, float dist, float dt)
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

    void HostileAISystem::updateMelee(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p,
                                      const Vector3D& target, float dist, float dt)
    {
        if (dist > p.attackRadius)
        {
            float rush = (st.kind == HostileKind::GhoulRush && dist < 2.3f) ? 1.35f : 1.0f;
            moveToward(gs, e, target, p.speed * rush, dt);
            return;
        }

        performAttack(gs, st, p);
    }

    void HostileAISystem::updateRanged(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p,
                                       const Vector3D& target, float dist, float dt)
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

    void HostileAISystem::performAttack(GameState& gs, HostileRuntimeState& st, const HostileProfile& p)
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


} // namespace bunker
