#include "world/WeatherSystem.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    void WeatherSystem::initializeRandom()
    {
        m_State = WeatherRuntimeState{};
        m_State.nextEventIn = 20.0f + random01() * 25.0f;
        recalculateDerivedValues();
    }

    void WeatherSystem::resetToClear()
    {
        m_State = WeatherRuntimeState{};
        recalculateDerivedValues();
    }

    void WeatherSystem::forceWeather(WeatherType type, float targetIntensity)
    {
        targetIntensity = std::clamp(targetIntensity, 0.0f, 1.0f);

        if (type == WeatherType::Clear || targetIntensity <= 0.01f)
        {
            m_State.targetIntensity = 0.0f;
            m_State.type = WeatherType::Clear;
            m_State.banner = "CLEAR";
            return;
        }

        m_State.current = type;
        m_State.type = type;
        m_State.targetIntensity = targetIntensity;
        m_State.banner = label();

        if (type == WeatherType::EtherStorm)
        {
            m_State.thunderTimer = 0.8f + random01() * 2.0f;
        }
    }

    void WeatherSystem::update(GameState& gs, float dt)
    {
        if (dt <= 0.0f)
            return;
        if (dt > 0.1f)
            dt = 0.1f;

        m_State.eventTimer += dt;
        m_State.damageTickTimer += dt;
        m_State.visualTimer += dt;

        if (m_State.eventTimer >= m_State.nextEventIn)
        {
            chooseNextWeather();
        }

        updateIntensity(dt);
        updateThunder(dt);
        recalculateDerivedValues();
        applyWorldEffects(gs, dt);

        if (m_State.damageTickTimer >= 0.50f)
        {
            applyDamageTick(gs, m_State.damageTickTimer);
            m_State.damageTickTimer = 0.0f;
        }
    }

    std::string WeatherSystem::label() const
    {
        switch (m_State.current)
        {
        case WeatherType::Clear:
            return "CLEAR";
        case WeatherType::EtherFog:
            return "ETHER FOG";
        case WeatherType::AcidRain:
            return "ACID RAIN";
        case WeatherType::AshStorm:
            return "ASH STORM";
        case WeatherType::EtherStorm:
            return "ETHER STORM";
        default:
            break;
        }
        return "UNKNOWN";
    }


} // namespace bunker
