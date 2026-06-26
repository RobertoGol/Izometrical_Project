#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Constants.hpp"
#include "gameplay/DamageSystem.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
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

    float intensity       = 0.0f;  // 0..1 текущая сила
    float targetIntensity = 0.0f;  // 0..1 куда плавно идём
    float transitionSpeed = 0.18f; // скорость изменения intensity в секунду

    float eventTimer      = 0.0f;
    float nextEventIn     = 35.0f;
    float damageTickTimer = 0.0f;
    float visualTimer     = 0.0f;

    // Вспышки EtherStorm. flashAlpha сам затухает, не висит навсегда.
    float thunderTimer    = 4.0f;
    float flashAlpha      = 0.0f;

    // Производные эффекты.
    float visibilityMultiplier = 1.0f; // 1.0 = видно нормально, 0.25 = почти слепота
    float sensorNoise          = 0.0f; // 0..1 разброс сенсоров/прицела
    float erosionBoost         = 0.0f; // усиление эфирной эрозии
    float acidDamagePerSecond  = 0.0f; // урон в секунду
    float floorSlickness       = 0.0f; // будущая физика скольжения
    std::string banner         = "CLEAR";
};

class WeatherSystem {
private:
    WeatherRuntimeState m_State;

public:
    WeatherSystem() = default;

    void initializeRandom() {
        m_State = WeatherRuntimeState{};
        m_State.nextEventIn = 20.0f + random01() * 25.0f;
        recalculateDerivedValues();
    }

    // Мгновенно очистить погоду. Удобно для загрузки/отладки.
    void resetToClear() {
        m_State = WeatherRuntimeState{};
        recalculateDerivedValues();
    }

    // Плавно включить погоду. Если type != Clear, тип меняется сразу,
    // а сила доходит до targetIntensity плавно. Это убирает старый баг:
    // "цель AcidRain, но эффекты всё ещё EtherFog".
    void forceWeather(WeatherType type, float targetIntensity) {
        targetIntensity = std::clamp(targetIntensity, 0.0f, 1.0f);

        if (type == WeatherType::Clear || targetIntensity <= 0.01f) {
            m_State.targetIntensity = 0.0f;
            m_State.type = WeatherType::Clear;
            m_State.banner = "CLEAR";
            return;
        }

        m_State.current = type;
        m_State.type = type;
        m_State.targetIntensity = targetIntensity;
        m_State.banner = label();

        if (type == WeatherType::EtherStorm) {
            m_State.thunderTimer = 0.8f + random01() * 2.0f;
        }
    }

    void update(GameState& gs, float dt) {
        if (dt <= 0.0f) return;
        if (dt > 0.1f) dt = 0.1f;

        m_State.eventTimer += dt;
        m_State.damageTickTimer += dt;
        m_State.visualTimer += dt;

        if (m_State.eventTimer >= m_State.nextEventIn) {
            chooseNextWeather();
        }

        updateIntensity(dt);
        updateThunder(dt);
        recalculateDerivedValues();
        applyWorldEffects(gs, dt);

        // Урон тикает редко, чтобы не зависеть от FPS и не дёргать HP каждый кадр.
        if (m_State.damageTickTimer >= 0.50f) {
            applyDamageTick(gs, m_State.damageTickTimer);
            m_State.damageTickTimer = 0.0f;
        }
    }

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

    std::string label() const {
        switch (m_State.current) {
            case WeatherType::Clear:      return "CLEAR";
            case WeatherType::EtherFog:   return "ETHER FOG";
            case WeatherType::AcidRain:   return "ACID RAIN";
            case WeatherType::AshStorm:   return "ASH STORM";
            case WeatherType::EtherStorm: return "ETHER STORM";
        }
        return "UNKNOWN";
    }

    // Overlay рисовать после мира/сущностей, но до HUD.
    void renderWorldOverlay(sf::RenderWindow& window) const {
        if (m_State.intensity <= 0.01f || m_State.current == WeatherType::Clear) return;

        sf::View oldView = window.getView();
        window.setView(window.getDefaultView());

        const float W = static_cast<float>(Config::SCREEN_WIDTH);
        const float H = static_cast<float>(Config::SCREEN_HEIGHT);
        const float I = std::clamp(m_State.intensity, 0.0f, 1.0f);

        if (m_State.current == WeatherType::EtherFog || m_State.current == WeatherType::EtherStorm) {
            drawFogOverlay(window, W, H, I);
        }

        if (m_State.current == WeatherType::AcidRain || m_State.current == WeatherType::EtherStorm) {
            drawAcidRainOverlay(window, W, H, I);
        }

        if (m_State.flashAlpha > 0.01f) {
            sf::RectangleShape flash({W, H});
            flash.setFillColor(sf::Color(215, 190, 255, static_cast<sf::Uint8>(std::clamp(m_State.flashAlpha, 0.0f, 160.0f))));
            window.draw(flash);
        }

        window.setView(oldView);
    }

    void renderHUD(sf::RenderWindow& window, const sf::Font* font) const {
        if (!font) return;

        sf::Text text;
        text.setFont(*font);
        text.setCharacterSize(13);
        text.setFillColor(hudColor());
        text.setPosition(12.0f, 96.0f);

        std::string line = "WEATHER: " + label() +
            "  INT " + std::to_string(static_cast<int>(m_State.intensity * 100.0f)) + "%";

        if (m_State.current == WeatherType::EtherFog) {
            line += "  VIS -" + std::to_string(static_cast<int>((1.0f - m_State.visibilityMultiplier) * 100.0f)) + "%";
        } else if (m_State.current == WeatherType::AcidRain) {
            line += "  ACID " + oneDecimal(m_State.acidDamagePerSecond) + "/s";
        } else if (m_State.current == WeatherType::EtherStorm) {
            line += "  VIS -" + std::to_string(static_cast<int>((1.0f - m_State.visibilityMultiplier) * 100.0f)) + "%";
            line += "  ACID " + oneDecimal(m_State.acidDamagePerSecond) + "/s";
        }

        text.setString(line);
        window.draw(text);
    }

private:
    void chooseNextWeather() {
        m_State.eventTimer = 0.0f;
        m_State.nextEventIn = 32.0f + random01() * 50.0f;

        const float roll = random01();
        WeatherType next = WeatherType::Clear;
        float target = 0.0f;

        if (roll < 0.42f) {
            next = WeatherType::Clear;
            target = 0.0f;
        } else if (roll < 0.68f) {
            next = WeatherType::EtherFog;
            target = 0.30f + random01() * 0.50f;
        } else if (roll < 0.90f) {
            next = WeatherType::AcidRain;
            target = 0.30f + random01() * 0.55f;
        } else {
            next = WeatherType::EtherStorm;
            target = 0.55f + random01() * 0.40f;
        }

        forceWeather(next, target);
        std::cout << "[WEATHER] incoming: " << weatherName(next)
                  << " intensity=" << static_cast<int>(target * 100.0f) << "%" << std::endl;
    }

    void updateIntensity(float dt) {
        const float diff = m_State.targetIntensity - m_State.intensity;
        const float step = m_State.transitionSpeed * dt;

        if (std::fabs(diff) <= step) {
            m_State.intensity = m_State.targetIntensity;
        } else {
            m_State.intensity += (diff > 0.0f ? step : -step);
        }

        m_State.intensity = std::clamp(m_State.intensity, 0.0f, 1.0f);

        // Если погода полностью стихла — реально Clear.
        if (m_State.targetIntensity <= 0.01f && m_State.intensity <= 0.01f) {
            m_State.current = WeatherType::Clear;
            m_State.intensity = 0.0f;
        }
    }

    void updateThunder(float dt) {
        if (m_State.flashAlpha > 0.0f) {
            m_State.flashAlpha = std::max(0.0f, m_State.flashAlpha - 260.0f * dt);
        }

        if (m_State.current != WeatherType::EtherStorm || m_State.intensity <= 0.10f) {
            return;
        }

        m_State.thunderTimer -= dt;
        if (m_State.thunderTimer <= 0.0f) {
            m_State.flashAlpha = 50.0f + 100.0f * m_State.intensity;
            m_State.thunderTimer = 2.8f + random01() * 5.5f;
        }
    }

    void recalculateDerivedValues() {
        const float I = std::clamp(m_State.intensity, 0.0f, 1.0f);

        m_State.visibilityMultiplier = 1.0f;
        m_State.sensorNoise = 0.0f;
        m_State.erosionBoost = 0.0f;
        m_State.acidDamagePerSecond = 0.0f;
        m_State.floorSlickness = 0.0f;

        switch (m_State.current) {
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
        }
        m_State.type = m_State.current;
        m_State.banner = label();
    }

    void applyWorldEffects(GameState& gs, float dt) {
        if (m_State.erosionBoost <= 0.001f) return;

        // Усиливаем уже заражённые тайлы. Не заражаем всю карту мгновенно.
        const float add = m_State.erosionBoost * 4.0f * dt;
        for (int x = 1; x < Config::MAP_WIDTH - 1; ++x) {
            for (int y = 1; y < Config::MAP_HEIGHT - 1; ++y) {
                if (gs.etherErosionMap[x][y] > 1.0f) {
                    gs.etherErosionMap[x][y] = std::min(100.0f, gs.etherErosionMap[x][y] + add);
                }
            }
        }
    }

    void applyDamageTick(GameState& gs, float elapsed) {
        if (m_State.acidDamagePerSecond <= 0.001f) return;

        const float dmg = m_State.acidDamagePerSecond * elapsed;

        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted) {
            gs.titan.health = std::max(0.0f, gs.titan.health - dmg * 0.45f);
            gs.titan.systems.tracksCondition = std::max(0.0f, gs.titan.systems.tracksCondition - dmg * 0.08f);
            gs.titan.systems.sensorLink = std::max(0.0f, gs.titan.systems.sensorLink - dmg * 0.06f);
            gs.titan.systems.turretStatus = std::max(0.0f, gs.titan.systems.turretStatus - dmg * 0.03f);
        } else {
            gs.playerHealth = std::max(0.0f, gs.playerHealth - dmg);
        }
    }

    void drawFogOverlay(sf::RenderWindow& window, float W, float H, float I) const {
        sf::RectangleShape fog({W, H});
        fog.setFillColor(sf::Color(115, 70, 180, static_cast<sf::Uint8>(30 + 95 * I)));
        window.draw(fog);

        // Несколько движущихся полос тумана. visualTimer не сбрасывается при смене погоды.
        for (int i = 0; i < 9; ++i) {
            const float phase = std::fmod(m_State.visualTimer * (14.0f + i * 1.7f) + i * 97.0f, H + 140.0f);
            const float y = phase - 70.0f;

            sf::RectangleShape band({W, 16.0f + 20.0f * I});
            band.setPosition(0.0f, y);
            band.setFillColor(sf::Color(180, 125, 255, static_cast<sf::Uint8>(15 + 30 * I)));
            window.draw(band);
        }
    }

    void drawAcidRainOverlay(sf::RenderWindow& window, float W, float H, float I) const {
        sf::RectangleShape tint({W, H});
        tint.setFillColor(sf::Color(70, 115, 30, static_cast<sf::Uint8>(18 + 55 * I)));
        window.draw(tint);

        const int drops = static_cast<int>(70 + 180 * I);
        const int Wi = std::max(1, static_cast<int>(W + 90.0f));
        const int Hi = std::max(1, static_cast<int>(H + 90.0f));
        const int tX = static_cast<int>(m_State.visualTimer * 235.0f);
        const int tY = static_cast<int>(m_State.visualTimer * 390.0f);

        for (int i = 0; i < drops; ++i) {
            const float x = static_cast<float>((i * 53 + tX) % Wi) - 45.0f;
            const float y = static_cast<float>((i * 89 + tY) % Hi) - 45.0f;

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, y), sf::Color(150, 255, 75, static_cast<sf::Uint8>(65 + 45 * I))),
                sf::Vertex(sf::Vector2f(x - 8.0f, y + 24.0f), sf::Color(150, 255, 75, 22))
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    sf::Color hudColor() const {
        switch (m_State.current) {
            case WeatherType::Clear:      return sf::Color(150, 210, 160);
            case WeatherType::EtherFog:   return sf::Color(210, 165, 255);
            case WeatherType::AcidRain:   return sf::Color(165, 255, 90);
            case WeatherType::EtherStorm: return sf::Color(235, 190, 255);
        }
        return sf::Color::White;
    }

    static std::string weatherName(WeatherType t) {
        switch (t) {
            case WeatherType::Clear:      return "CLEAR";
            case WeatherType::EtherFog:   return "ETHER FOG";
            case WeatherType::AcidRain:   return "ACID RAIN";
            case WeatherType::EtherStorm: return "ETHER STORM";
        }
        return "UNKNOWN";
    }

    static std::string oneDecimal(float v) {
        int scaled = static_cast<int>(v * 10.0f + 0.5f);
        return std::to_string(scaled / 10) + "." + std::to_string(std::abs(scaled % 10));
    }

    static float random01() {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }
};

}  // namespace bunker
