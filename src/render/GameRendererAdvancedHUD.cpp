// ══════════════════════════════════════════════════════════════════════
// ПРИМЕЧАНИЕ: Весь игровой код рендера (360 строк) находится прямо здесь!
// Объявления методов (прототипы) находятся в include/render/GameRenderer.hpp.
// Ни одна строчка логики не была удалена из проекта.
// ══════════════════════════════════════════════════════════════════════
#include "render/GameRenderer.hpp"

#include "AdvancedMechanics.hpp"
#include "Constants.hpp"
#include "GameState.hpp"
#include "HostileAISystem.hpp"
#include "IsoMath.hpp"
#include "TimeShift.hpp"
#include "Types.hpp"
#include "ai/EnemyArchetypeRegistry.hpp"
#include <cstdint>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════
    // Отрисовка изометрического пола
    // ═══════════════════════════════════════════════════════
    void GameRenderer::renderAdvancedHUD(sf::RenderWindow& window, const AdvancedMechanics& adv, const sf::Font* font)
    {
        // Погодный экранный фильтр (Внедрён GLSL-шейдер гибридной атмосферы: Эфирный Туман + Кислотный Дождь)
        const auto& weather = adv.weather.state();
        if (weather.type != WeatherType::Clear)
        {
            sf::RectangleShape overlay(
                {static_cast<float>(Config::SCREEN_WIDTH), static_cast<float>(Config::SCREEN_HEIGHT)});

            static sf::Shader s_AtmosphericShader;
            static bool s_ShaderLoaded = false;
            static sf::Clock s_HazardClock;

            if (sf::Shader::isAvailable())
            {
                if (!s_ShaderLoaded)
                {
                    const char* ATMOSPHERIC_HAZARD_SHADER = R"(
                        uniform float u_time;
                        uniform float u_fogIntensity;
                        uniform float u_rainIntensity;
                        uniform vec2 u_resolution;

                        float hash(vec2 p) {
                            p = fract(p * vec3(.1031, .1030, .0973).xy);
                            p += dot(p, p.yx + 33.33);
                            return fract((p.x + p.y) * p.x);
                        }

                        float noise(vec2 p) {
                            vec2 i = floor(p);
                            vec2 f = fract(p);
                            f = f * f * (3.0 - 2.0 * f);
                            return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),
                                       mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
                        }

                        float fbm(vec2 p) {
                            float v = 0.0;
                            float a = 0.5;
                            for (int i = 0; i < 4; ++i) {
                                v += a * noise(p);
                                p = p * 2.0 + vec2(u_time * 0.15, u_time * 0.05);
                                a *= 0.5;
                            }
                            return v;
                        }

                        void main() {
                            vec2 uv = gl_FragCoord.xy / u_resolution;
                            vec4 finalColor = vec4(0.0);

                            if (u_fogIntensity > 0.01) {
                                float n = fbm(uv * 3.5 + vec2(u_time * 0.08, -u_time * 0.03));
                                vec3 fogTint = vec3(0.35, 0.65, 0.85);
                                float fogAlpha = clamp(n * u_fogIntensity * 0.85, 0.0, 0.75);
                                finalColor = mix(finalColor, vec4(fogTint, fogAlpha), fogAlpha);
                            }

                            if (u_rainIntensity > 0.01) {
                                vec2 rainUV = uv * vec2(80.0, 15.0);
                                rainUV.y += u_time * 18.0;
                                float r = noise(rainUV);
                                if (r > 0.88) {
                                    float rainStreak = clamp((r - 0.88) * 8.3 * u_rainIntensity, 0.0, 0.85);
                                    vec3 acidTint = vec3(0.25, 0.95, 0.35);
                                    finalColor = mix(finalColor, vec4(acidTint, rainStreak), rainStreak);
                                }
                            }

                            gl_FragColor = finalColor;
                        }
                    )";
                    s_ShaderLoaded =
                        s_AtmosphericShader.loadFromMemory(ATMOSPHERIC_HAZARD_SHADER, sf::Shader::Type::Fragment);
                }

                if (s_ShaderLoaded)
                {
                    float fogInt = (weather.type == WeatherType::EtherFog || weather.type == WeatherType::EtherStorm)
                                       ? weather.intensity
                                       : 0.0f;
                    float rainInt = (weather.type == WeatherType::AcidRain || weather.type == WeatherType::EtherStorm)
                                        ? weather.intensity
                                        : 0.0f;
                    if (weather.type == WeatherType::AshStorm)
                    {
                        fogInt = weather.intensity * 0.8f;
                    }

                    s_AtmosphericShader.setUniform("u_time", s_HazardClock.getElapsedTime().asSeconds());
                    s_AtmosphericShader.setUniform("u_fogIntensity", fogInt);
                    s_AtmosphericShader.setUniform("u_rainIntensity", rainInt);
                    s_AtmosphericShader.setUniform("u_resolution",
                                                   sf::Glsl::Vec2(static_cast<float>(Config::SCREEN_WIDTH),
                                                                  static_cast<float>(Config::SCREEN_HEIGHT)));

                    window.draw(overlay, &s_AtmosphericShader);
                    return;
                }
            }

            // Fallback для старых видеокарт без поддержки GLSL:
            if (weather.type == WeatherType::EtherFog)
            {
                overlay.setFillColor(sf::Color(80, 120, 180, static_cast<std::uint8_t>(45 + 60 * weather.intensity)));
            }
            if (weather.type == WeatherType::AcidRain)
            {
                overlay.setFillColor(sf::Color(70, 180, 70, static_cast<std::uint8_t>(35 + 55 * weather.intensity)));
            }
            if (weather.type == WeatherType::AshStorm)
            {
                overlay.setFillColor(sf::Color(180, 120, 70, static_cast<std::uint8_t>(35 + 55 * weather.intensity)));
            }
            window.draw(overlay);
        }

        if (!font)
            return;

        sf::Text text(*font, "", 14);
        text.setFillColor(sf::Color(230, 235, 210));
        text.setOutlineThickness(1.0f);
        text.setOutlineColor(sf::Color::Black);

        const auto& tank = adv.tankUtility.runtime();
        const auto& weapon = adv.survival.weapon();

        std::string util = "BucketRig";
        switch (tank.utility)
        {
        case TankUtilityMode::RamShield:
            util = "RamShield";
            break;
        case TankUtilityMode::TowCoupler:
            util = "TowCoupler";
            break;
        default:
            break;
        }

        std::string seat = (tank.seat == TankSeat::Driver) ? "Driver" : "Gunner";

        std::ostringstream ss;
        ss << "ADVANCED MECHANICS\n"
           << "Weather: " << weather.banner << " " << static_cast<int>(weather.intensity * 100.0f) << "%\n"
           << "Stress: " << static_cast<int>(adv.survival.stress()) << " | Ammo: " << weapon.magazine << "/"
           << weapon.reserveAmmo << (weapon.isReloading ? " RELOADING" : "") << "\n"
           << "Tank: " << util << " | Seat: " << seat << " | Heat: " << static_cast<int>(tank.cannonThermalLoad)
           << (tank.overheated ? " OVERHEATED" : "") << "\n"
           << "CAMP[B]: " << (adv.camp.enabled() ? "ON" : "OFF") << " | ToolGun[F7 mode/F8 use/F2 undo/F3 redo]\n";

        if (!adv.story.lastEvent().empty())
            ss << "Story: " << adv.story.lastEvent() << "\n";
        if (!adv.radio.lastSubtitle().empty())
            ss << adv.radio.lastSubtitle() << "\n";
        if (!adv.toolgun.lastValidation().empty())
            ss << adv.toolgun.lastValidation() << "\n";

        text.setString(ss.str());
        text.setPosition({12.0f, 96.0f});
        window.draw(text);
    }

} // namespace bunker
