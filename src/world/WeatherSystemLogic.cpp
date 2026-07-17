#include "world/WeatherSystem.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    void WeatherSystem::chooseNextWeather()
    {
        m_State.eventTimer = 0.0f;
        m_State.nextEventIn = 32.0f + random01() * 50.0f;

        const float roll = random01();
        WeatherType next = WeatherType::Clear;
        float target = 0.0f;

        if (roll < 0.42f)
        {
            next = WeatherType::Clear;
            target = 0.0f;
        }
        else if (roll < 0.68f)
        {
            next = WeatherType::EtherFog;
            target = 0.30f + random01() * 0.50f;
        }
        else if (roll < 0.90f)
        {
            next = WeatherType::AcidRain;
            target = 0.30f + random01() * 0.55f;
        }
        else
        {
            next = WeatherType::EtherStorm;
            target = 0.55f + random01() * 0.40f;
        }

        forceWeather(next, target);
        bunker::logInfo() << "[WEATHER] incoming: " << weatherName(next)
                          << " intensity=" << static_cast<int>(target * 100.0f) << "%" << std::endl;
    }

    void WeatherSystem::updateIntensity(float dt)
    {
        const float diff = m_State.targetIntensity - m_State.intensity;
        const float step = m_State.transitionSpeed * dt;

        if (std::fabs(diff) <= step)
        {
            m_State.intensity = m_State.targetIntensity;
        }
        else
        {
            m_State.intensity += (diff > 0.0f ? step : -step);
        }

        m_State.intensity = std::clamp(m_State.intensity, 0.0f, 1.0f);

        if (m_State.targetIntensity <= 0.01f && m_State.intensity <= 0.01f)
        {
            m_State.current = WeatherType::Clear;
            m_State.intensity = 0.0f;
        }
    }

    void WeatherSystem::updateThunder(float dt)
    {
        if (m_State.flashAlpha > 0.0f)
        {
            m_State.flashAlpha = std::max(0.0f, m_State.flashAlpha - 260.0f * dt);
        }

        if (m_State.current != WeatherType::EtherStorm || m_State.intensity <= 0.10f)
        {
            return;
        }

        m_State.thunderTimer -= dt;
        if (m_State.thunderTimer <= 0.0f)
        {
            m_State.flashAlpha = 50.0f + 100.0f * m_State.intensity;
            m_State.thunderTimer = 2.8f + random01() * 5.5f;
        }
    }

    void WeatherSystem::recalculateDerivedValues()
    {
        const float I = std::clamp(m_State.intensity, 0.0f, 1.0f);

        m_State.visibilityMultiplier = 1.0f;
        m_State.sensorNoise = 0.0f;
        m_State.erosionBoost = 0.0f;
        m_State.acidDamagePerSecond = 0.0f;
        m_State.floorSlickness = 0.0f;

        switch (m_State.current)
        {
        case WeatherType::Clear:
            break;

        case WeatherType::EtherFog:
            m_State.visibilityMultiplier = std::clamp(1.0f - I * 0.55f, 0.35f, 1.0f);
            m_State.sensorNoise = I * 0.42f;
            m_State.erosionBoost = I * 0.35f;
            break;

        case WeatherType::AcidRain:
            m_State.visibilityMultiplier = std::clamp(1.0f - I * 0.18f, 0.72f, 1.0f);
            m_State.sensorNoise = I * 0.12f;
            m_State.acidDamagePerSecond = I * 1.6f;
            m_State.floorSlickness = I * 0.55f;
            break;

        case WeatherType::AshStorm:
        case WeatherType::EtherStorm:
            m_State.visibilityMultiplier = std::clamp(1.0f - I * 0.70f, 0.25f, 1.0f);
            m_State.sensorNoise = I * 0.90f;
            m_State.erosionBoost = I * 0.75f;
            m_State.acidDamagePerSecond = I * 1.15f;
            m_State.floorSlickness = I * 0.35f;
            break;
        default:
            break;
        }
        m_State.type = m_State.current;
        m_State.banner = label();
    }


} // namespace bunker
