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
        default:
            break;
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
        default:
            break;
        }
        return Config::ENEMY_RADIUS;
    }

    void HostileAISystem::ensureStateSize(const GameState& gs)
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

    void HostileAISystem::applyMechanicalDegradation(HostileProfile& p, const HostileRuntimeState& st, Enemy& e) const
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


} // namespace bunker
