#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 5) REACTIVE WORLD SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    int ReactiveWorldSystem::add(BreakableKind kind, Vector3D pos)
    {
        BreakableObject b;
        b.id = ++m_NextId;
        b.kind = kind;
        b.position = pos;
        switch (kind)
        {
        case BreakableKind::Glass:
            b.health = 8.0f;
            b.radius = 0.30f;
            break;
        case BreakableKind::Vegetation:
            b.health = 12.0f;
            b.radius = 0.45f;
            break;
        case BreakableKind::Crate:
            b.health = 35.0f;
            b.radius = 0.42f;
            break;
        case BreakableKind::Barrel:
            b.health = 25.0f;
            b.radius = 0.40f;
            break;
        case BreakableKind::Console:
            b.health = 55.0f;
            b.radius = 0.55f;
            break;
        default:
            break;
        }
        m_Breakables.push_back(b);
        return b.id;
    }

    void ReactiveWorldSystem::seedDefault()
    {
        if (!m_Breakables.empty())
        {
            return;
        }
        add(BreakableKind::Crate, {7.0f, 5.0f, 0.0f});
        add(BreakableKind::Glass, {9.0f, 8.0f, 0.0f});
        add(BreakableKind::Barrel, {12.0f, 9.0f, 0.0f});
        add(BreakableKind::Console, {4.0f, 13.0f, 0.0f});
        add(BreakableKind::Vegetation, {15.0f, 15.0f, 0.0f});
    }

    void ReactiveWorldSystem::update(GameState& gs, float dt)
    {
        for (auto& wave : m_Waves)
        {
            wave.ttl -= dt;
            wave.radius += (wave.maxRadius / 0.45f) * dt;
            applyWave(gs, wave, dt);
        }
        m_Waves.erase(std::remove_if(m_Waves.begin(), m_Waves.end(), [](const ShockWave& w) { return w.ttl <= 0.0f; }),
                      m_Waves.end());

        for (auto& b : m_Breakables)
        {
            if (b.broken)
            {
                continue;
            }
            b.position += b.velocity * dt;
            b.velocity = b.velocity * std::pow(0.04f, dt);
            if (b.kind == BreakableKind::Barrel && b.health <= 0.0f)
            {
                explodeBarrel(gs, b);
            }
            else if (b.health <= 0.0f)
            {
                b.broken = true;
            }
        }
    }

    void ReactiveWorldSystem::damageAt(GameState& gs, Vector3D pos, float radius, float damage, float impulse)
    {
        for (auto& b : m_Breakables)
        {
            if (b.broken)
            {
                continue;
            }
            const float d = advDist2D(pos, b.position);
            if (d <= radius + b.radius)
            {
                const float falloff = 1.0f - advClamp(d / std::max(radius, 0.01f), 0.0f, 1.0f);
                b.health -= damage * std::max(0.25f, falloff);
                Vector3D dir = advNormalize2D(b.position - pos);
                b.velocity += dir * (impulse * falloff);
            }
        }
        m_Waves.push_back({pos, 0.1f, radius * 1.7f, impulse, 0.45f});
        (void)gs;
    }

    void ReactiveWorldSystem::explodeBarrel(GameState& gs, BreakableObject& b)
    {
        b.broken = true;
        damageAt(gs, b.position, 2.8f, 75.0f, 5.5f);
        for (auto& e : gs.enemies)
        {
            if (e.isAlive && advDistSq(e.position, b.position) < 2.8f * 2.8f)
            {
                if (DamageSystem::applyEnemyDamage(gs, e, 90.0f, DamageType::Explosive))
                {
                    gs.score += 50;
                }
            }
        }
    }

    void ReactiveWorldSystem::applyWave(GameState& gs, const ShockWave& wave, float dt)
    {
        const float inner = std::max(0.0f, wave.radius - 0.35f);
        const float outer = wave.radius;
        const float dp = advDist2D(gs.playerPos, wave.origin);
        if (dp >= inner && dp <= outer)
        {
            gs.playerPos += advNormalize2D(gs.playerPos - wave.origin) * (wave.force * dt);
        }
        for (auto& e : gs.enemies)
        {
            if (!e.isAlive)
            {
                continue;
            }
            const float de = advDist2D(e.position, wave.origin);
            if (de >= inner && de <= outer)
            {
                e.position += advNormalize2D(e.position - wave.origin) * (wave.force * 0.8f * dt);
            }
        }
    }


} // namespace bunker
