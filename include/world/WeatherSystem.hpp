#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "core/Constants.hpp"
#include "gameplay/DamageSystem.hpp"
#include <SFML/Graphics.hpp>
#include <string>

namespace bunker {

enum class WeatherType {
    Clear,
    EtherFog,
    AcidRain,
    AshStorm,
    EtherStorm
};

struct WeatherRuntimeState {
    WeatherType current = WeatherType::Clear;
    WeatherType type    = WeatherType::Clear;
    float intensity       = 0.0f;
    float targetIntensity = 0.0f;
    float transitionSpeed = 0.18f;
    float eventTimer      = 0.0f;
    float nextEventIn     = 35.0f;
    float damageTickTimer = 0.0f;
    float visualTimer     = 0.0f;
    float thunderTimer    = 4.0f;
    float flashAlpha      = 0.0f;
    float visibilityMultiplier = 1.0f;
    float sensorNoise          = 0.0f;
    float erosionBoost         = 0.0f;
    float acidDamagePerSecond  = 0.0f;
    float floorSlickness       = 0.0f;
    std::string banner         = "CLEAR";
};

class WeatherSystem {
private:
    WeatherRuntimeState m_State;

public:
    WeatherSystem() = default;

    void initializeRandom();
    void resetToClear();
    void forceWeather(WeatherType type, float targetIntensity);
    void update(GameState& gs, float dt);
    void renderWorldOverlay(sf::RenderWindow& window) const;
    void renderHUD(sf::RenderWindow& window, const sf::Font* font) const;

    float visibilityMultiplier() const { return m_State.visibilityMultiplier; }
    float enemyVisionMultiplier() const { return m_State.visibilityMultiplier; }
    float bulletSpreadPenalty() const { return (m_State.current == WeatherType::AshStorm || m_State.current == WeatherType::EtherStorm) ? 0.12f * m_State.intensity : 0.0f; }
    float sensorNoise() const { return m_State.sensorNoise; }
    float erosionBoost() const { return m_State.erosionBoost; }
    float acidDamagePerSecond() const { return m_State.acidDamagePerSecond; }
    float floorSlickness() const { return m_State.floorSlickness; }
    float intensity() const { return m_State.intensity; }
    WeatherType currentWeather() const { return m_State.current; }
    const WeatherRuntimeState& state() const { return m_State; }

    bool isDangerous() const {
        return m_State.current == WeatherType::AcidRain ||
               m_State.current == WeatherType::AshStorm ||
               m_State.current == WeatherType::EtherStorm;
    }

    std::string label() const;

private:
    void chooseNextWeather();
    void updateIntensity(float dt);
    void updateThunder(float dt);
    void recalculateDerivedValues();
    void applyWorldEffects(GameState& gs, float dt);
    void applyDamageTick(GameState& gs, float elapsed);
    void drawFogOverlay(sf::RenderWindow& window, float W, float H, float I) const;
    void drawAcidRainOverlay(sf::RenderWindow& window, float W, float H, float I) const;
    sf::Color hudColor() const;
    static std::string weatherName(WeatherType t);
    static std::string oneDecimal(float v);
    static float random01();
};

}  // namespace bunker
