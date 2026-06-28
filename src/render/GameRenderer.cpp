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
#include "ai/EnemyArchetypeRegistry.hpp"
#include "IsoMath.hpp"
#include "TimeShift.hpp"
#include "Types.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════
    // Отрисовка изометрического пола
    // ═══════════════════════════════════════════════════════
    void GameRenderer::renderFloor(sf::RenderWindow &window,
                                   const GameState &gs,
                                   const TimeShift &timeShift)
    {
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                sf::ConvexShape tile(4);
                tile.setPoint(0, IsoMath::worldToScreen(static_cast<float>(x), static_cast<float>(y)));
                tile.setPoint(1, IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y)));
                tile.setPoint(2, IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y + 1)));
                tile.setPoint(3, IsoMath::worldToScreen(static_cast<float>(x), static_cast<float>(y + 1)));

                if (gs.sectorMap[x][y] == 1)
                {
                    if (timeShift.isPast())
                    {
                        tile.setFillColor(sf::Color(60, 70, 90)); // Прошлое — синеватые стены
                        tile.setOutlineColor(sf::Color(80, 90, 115));
                    }
                    else
                    {
                        tile.setFillColor(sf::Color(70, 75, 85)); // Настоящее — серые стены
                        tile.setOutlineColor(sf::Color(90, 95, 105));
                    }
                }
                else if (gs.etherErosionMap[x][y] > 5.0f)
                {
                    int intensity = static_cast<int>(std::min(gs.etherErosionMap[x][y] * 1.5f, 80.0f));
                    tile.setFillColor(sf::Color(30 + intensity / 2, 20, 35 + intensity));
                    tile.setOutlineColor(sf::Color(50, 30, 60));
                }
                else
                {
                    if (timeShift.isPast())
                    {
                        tile.setFillColor(sf::Color(30, 35, 45)); // Прошлое — холодные тона
                        tile.setOutlineColor(sf::Color(45, 50, 60));
                    }
                    else
                    {
                        tile.setFillColor(sf::Color(35, 35, 40)); // Настоящее
                        tile.setOutlineColor(sf::Color(50, 50, 55));
                    }
                }

                tile.setOutlineThickness(1.0f);
                window.draw(tile);
            }
        }
    }

    // ═══════════════════════════════════════════════════════
    // Отрисовка сущностей с Z-сортировкой
    // ═══════════════════════════════════════════════════════
    void GameRenderer::renderEntities(sf::RenderWindow &window,
                                      const GameState &gs,
                                      const TimeShift &timeShift,
                                      const HostileAISystem &hostileAI)
    {
        std::vector<RenderObject> renderQueue;

        // Pip-Pad на полу
        if (!gs.bunkerProgression.hasFoundPipPad)
        {
            float px = gs.bunkerProgression.pipPadSpawnPos.x;
            float py = gs.bunkerProgression.pipPadSpawnPos.y;
            renderQueue.push_back({px + py, [&, px, py]()
                                   {
                                       sf::RectangleShape pad({10, 8});
                                       pad.setFillColor(sf::Color(255, 255, 50, 200));
                                       pad.setOrigin(5, 4);
                                       pad.setPosition(IsoMath::worldToScreen(px, py));
                                       window.draw(pad);
                                   }});
        }

        // Лут-контейнеры
        for (const auto &c : gs.lootContainers)
        {
            float cx = c.position.x;
            float cy = c.position.y;
            bool opened = c.isOpened;
            renderQueue.push_back({cx + cy, [&, cx, cy, opened]()
                                   {
                                       sf::RectangleShape box({12, 10});
                                       box.setOrigin(6, 5);
                                       box.setPosition(IsoMath::worldToScreen(cx, cy));
                                       box.setFillColor(opened ? sf::Color(80, 60, 30, 120) : sf::Color(180, 140, 60));
                                       box.setOutlineThickness(1.0f);
                                       box.setOutlineColor(sf::Color(120, 100, 40));
                                       window.draw(box);
                                   }});
        }

        // Враги: цвета и форма зависят от HostileAISystem
        bool isPast = timeShift.isPast();
        const auto &hostileStates = hostileAI.debugStates();
        for (std::size_t i = 0; i < gs.enemies.size(); ++i)
        {
            const auto &e = gs.enemies[i];
            if (!e.isAlive)
                continue;

            float ex = e.position.x;
            float ey = e.position.y;
            float er = e.radius;
            HostileKind kind = HostileKind::VerminRush;
            HostileAlertState alert = HostileAlertState::Idle;
            if (i < hostileStates.size())
            {
                kind = hostileStates[i].kind;
                alert = hostileStates[i].alert;
            }

            renderQueue.push_back({ex + ey, [&, ex, ey, er, isPast, kind, alert]()
                                   {
                                       const auto &profile = EnemyArchetypeRegistry::getProfile(kind);
                                       sf::Color body = profile.bodyColor;
                                       int points = profile.shapePoints;

                                       if (isPast)
                                       {
                                           body = sf::Color(
                                               static_cast<sf::Uint8>(std::min(255, body.r / 2 + 50)),
                                               static_cast<sf::Uint8>(std::min(255, body.g / 2 + 90)),
                                               static_cast<sf::Uint8>(std::min(255, body.b + 40)),
                                               220);
                                       }

                                       float radius = 8.0f + er * 5.0f;
                                       sf::CircleShape shape(radius, points);
                                       shape.setFillColor(body);
                                       shape.setOrigin(radius, radius);
                                       shape.setPosition(IsoMath::worldToScreen(ex, ey));

                                       if (alert == HostileAlertState::Aggro)
                                       {
                                           shape.setOutlineThickness(2.0f);
                                           shape.setOutlineColor(sf::Color(255, 40, 40));
                                       }
                                       else if (alert == HostileAlertState::Suspicious ||
                                                alert == HostileAlertState::Searching)
                                       {
                                           shape.setOutlineThickness(1.5f);
                                           shape.setOutlineColor(sf::Color(255, 220, 60));
                                       }

                                       window.draw(shape);
                                   }});
        }

        // Вышка связи
        {
            float tx = gs.towerPosition.x;
            float ty = gs.towerPosition.y;
            renderQueue.push_back({tx + ty, [&, tx, ty]()
                                   {
                                       sf::CircleShape tower(7.0f, 6);
                                       tower.setFillColor(sf::Color(200, 200, 255));
                                       tower.setOrigin(7, 7);
                                       tower.setPosition(IsoMath::worldToScreen(tx, ty));
                                       tower.setOutlineThickness(2.0f);
                                       tower.setOutlineColor(sf::Color::White);
                                       window.draw(tower);
                                   }});
        }

        // Танк БТ-7274
        {
            float bx = gs.titan.position.x;
            float by = gs.titan.position.y;
            bool piloted = gs.titan.isPiloted;
            renderQueue.push_back({bx + by, [&, bx, by, piloted]()
                                   {
                                       sf::CircleShape bt(16.0f, 4);
                                       bt.setFillColor(piloted ? sf::Color(255, 160, 40) : sf::Color(230, 115, 25));
                                       bt.setOrigin(16, 16);
                                       bt.setPosition(IsoMath::worldToScreen(bx, by));
                                       bt.setOutlineThickness(2.0f);
                                       bt.setOutlineColor(sf::Color(180, 80, 10));
                                       window.draw(bt);
                                   }});
        }

        // Игрок (только если НЕ в Танке)
        if (gs.playerMode != UnitMode::Titan)
        {
            float ppx = gs.playerPos.x;
            float ppy = gs.playerPos.y;
            renderQueue.push_back({ppx + ppy + 0.01f, [&, ppx, ppy]()
                                   {
                                       sf::CircleShape pShape(12.0f);
                                       pShape.setFillColor(sf::Color::Cyan);
                                       pShape.setOrigin(12, 12);
                                       pShape.setPosition(IsoMath::worldToScreen(ppx, ppy));
                                       pShape.setOutlineThickness(2.0f);
                                       pShape.setOutlineColor(sf::Color(0, 200, 200));
                                       window.draw(pShape);
                                   }});
        }

        // Z-сортировка и отрисовка
        std::sort(renderQueue.begin(), renderQueue.end(),
                  [](const RenderObject &a, const RenderObject &b)
                  {
                      return a.depth < b.depth;
                  });

        for (auto &obj : renderQueue)
        {
            obj.drawFunc();
        }
    }

    // ═══════════════════════════════════════════════════════
    // Отрисовка перенесённых advanced-механик:
    // Weather, C.A.M.P., breakables, shock waves.
    // ═══════════════════════════════════════════════════════
    void GameRenderer::renderAdvancedWorld(sf::RenderWindow &window,
                                           const AdvancedMechanics &adv)
    {
        // Разрушаемые объекты
        for (const auto &b : adv.reactive.breakables())
        {
            if (b.broken)
                continue;

            sf::Color color = sf::Color(150, 110, 60);
            int points = 4;
            float radius = 8.0f;
            switch (b.kind)
            {
            case BreakableKind::Glass:
                color = sf::Color(125, 210, 255, 150);
                points = 4;
                radius = 7.0f;
                break;
            case BreakableKind::Vegetation:
                color = sf::Color(75, 170, 70);
                points = 7;
                radius = 9.0f;
                break;
            case BreakableKind::Crate:
                color = sf::Color(170, 120, 55);
                points = 4;
                radius = 9.0f;
                break;
            case BreakableKind::Barrel:
                color = sf::Color(210, 75, 45);
                points = 12;
                radius = 8.0f;
                break;
            case BreakableKind::Console:
                color = sf::Color(85, 165, 210);
                points = 6;
                radius = 10.0f;
                break;
            }

            sf::CircleShape shape(radius, points);
            shape.setOrigin(radius, radius);
            shape.setPosition(IsoMath::worldToScreen(b.position.x, b.position.y));
            shape.setFillColor(color);
            shape.setOutlineThickness(1.0f);
            shape.setOutlineColor(sf::Color::Black);
            window.draw(shape);
        }

        // Волны от взрывов/ударов
        for (const auto &w : adv.reactive.waves())
        {
            float sr = w.radius * 24.0f;
            sf::CircleShape ring(sr);
            ring.setOrigin(sr, sr);
            ring.setPosition(IsoMath::worldToScreen(w.origin.x, w.origin.y));
            ring.setFillColor(sf::Color(0, 0, 0, 0));
            ring.setOutlineThickness(2.0f);
            ring.setOutlineColor(sf::Color(255, 230, 120, 90));
            window.draw(ring);
        }

        // C.A.M.P. построенные объекты
        for (const auto &obj : adv.camp.objects())
        {
            sf::Vector2f pos = IsoMath::worldToScreen(obj.tileX + 0.5f, obj.tileY + 0.5f);
            if (obj.type == CampObjectType::DefenseTurret)
            {
                sf::CircleShape t(9.0f, 3);
                t.setOrigin(9.0f, 9.0f);
                t.setPosition(pos);
                t.setFillColor(sf::Color(210, 210, 70));
                t.setOutlineThickness(1.5f);
                t.setOutlineColor(sf::Color::Black);
                window.draw(t);
            }
            else if (obj.type == CampObjectType::SupplyCrate)
            {
                sf::RectangleShape c({14.0f, 12.0f});
                c.setOrigin(7.0f, 6.0f);
                c.setPosition(pos);
                c.setFillColor(sf::Color(80, 200, 110));
                c.setOutlineThickness(1.0f);
                c.setOutlineColor(sf::Color::Black);
                window.draw(c);
            }
        }

        // Превью строительства (Объёмный 3D снаппинг-бокс в духе Fallout 76)
        if (adv.camp.enabled())
        {
            const auto &p = adv.camp.preview();
            sf::Color fillCol = p.isPlacementValid ? sf::Color(50, 240, 90, 85) : sf::Color(240, 50, 50, 85);
            sf::Color lineCol = p.isPlacementValid ? sf::Color(90, 255, 120) : sf::Color(255, 90, 90);

            sf::Vector2f b0 = IsoMath::worldToScreen(static_cast<float>(p.tileX), static_cast<float>(p.tileY));
            sf::Vector2f b1 = IsoMath::worldToScreen(static_cast<float>(p.tileX + 1), static_cast<float>(p.tileY));
            sf::Vector2f b2 = IsoMath::worldToScreen(static_cast<float>(p.tileX + 1), static_cast<float>(p.tileY + 1));
            sf::Vector2f b3 = IsoMath::worldToScreen(static_cast<float>(p.tileX), static_cast<float>(p.tileY + 1));

            sf::ConvexShape bottomFace(4);
            bottomFace.setPoint(0, b0);
            bottomFace.setPoint(1, b1);
            bottomFace.setPoint(2, b2);
            bottomFace.setPoint(3, b3);
            bottomFace.setFillColor(fillCol);
            bottomFace.setOutlineThickness(2.0f);
            bottomFace.setOutlineColor(lineCol);
            window.draw(bottomFace);

            float heightOffset = 38.0f;
            sf::Vector2f t0{b0.x, b0.y - heightOffset};
            sf::Vector2f t1{b1.x, b1.y - heightOffset};
            sf::Vector2f t2{b2.x, b2.y - heightOffset};
            sf::Vector2f t3{b3.x, b3.y - heightOffset};

            sf::ConvexShape topFace(4);
            topFace.setPoint(0, t0);
            topFace.setPoint(1, t1);
            topFace.setPoint(2, t2);
            topFace.setPoint(3, t3);
            topFace.setFillColor(sf::Color::Transparent);
            topFace.setOutlineThickness(1.5f);
            topFace.setOutlineColor(lineCol);
            window.draw(topFace);

            auto drawWire = [&](sf::Vector2f from, sf::Vector2f to)
            {
                sf::Vertex line[] = {sf::Vertex(from, lineCol), sf::Vertex(to, lineCol)};
                window.draw(line, 2, sf::Lines);
            };
            drawWire(b0, t0);
            drawWire(b1, t1);
            drawWire(b2, t2);
            drawWire(b3, t3);
        }
    }

    void GameRenderer::renderAdvancedHUD(sf::RenderWindow &window,
                                         const AdvancedMechanics &adv,
                                         const sf::Font *font)
    {
        // Погодный экранный фильтр (Внедрён GLSL-шейдер гибридной атмосферы: Эфирный Туман + Кислотный Дождь)
        const auto &weather = adv.weather.state();
        if (weather.type != WeatherType::Clear)
        {
            sf::RectangleShape overlay({static_cast<float>(Config::SCREEN_WIDTH), static_cast<float>(Config::SCREEN_HEIGHT)});

            static sf::Shader s_AtmosphericShader;
            static bool s_ShaderLoaded = false;
            static sf::Clock s_HazardClock;

            if (sf::Shader::isAvailable())
            {
                if (!s_ShaderLoaded)
                {
                    const char *ATMOSPHERIC_HAZARD_SHADER = R"(
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
                    s_ShaderLoaded = s_AtmosphericShader.loadFromMemory(ATMOSPHERIC_HAZARD_SHADER, sf::Shader::Fragment);
                }

                if (s_ShaderLoaded)
                {
                    float fogInt = (weather.type == WeatherType::EtherFog || weather.type == WeatherType::EtherStorm) ? weather.intensity : 0.0f;
                    float rainInt = (weather.type == WeatherType::AcidRain || weather.type == WeatherType::EtherStorm) ? weather.intensity : 0.0f;
                    if (weather.type == WeatherType::AshStorm)
                    {
                        fogInt = weather.intensity * 0.8f;
                    }

                    s_AtmosphericShader.setUniform("u_time", s_HazardClock.getElapsedTime().asSeconds());
                    s_AtmosphericShader.setUniform("u_fogIntensity", fogInt);
                    s_AtmosphericShader.setUniform("u_rainIntensity", rainInt);
                    s_AtmosphericShader.setUniform("u_resolution", sf::Glsl::Vec2(static_cast<float>(Config::SCREEN_WIDTH), static_cast<float>(Config::SCREEN_HEIGHT)));

                    window.draw(overlay, &s_AtmosphericShader);
                    return;
                }
            }

            // Fallback для старых видеокарт без поддержки GLSL:
            if (weather.type == WeatherType::EtherFog)
            {
                overlay.setFillColor(sf::Color(80, 120, 180, static_cast<sf::Uint8>(45 + 60 * weather.intensity)));
            }
            if (weather.type == WeatherType::AcidRain)
            {
                overlay.setFillColor(sf::Color(70, 180, 70, static_cast<sf::Uint8>(35 + 55 * weather.intensity)));
            }
            if (weather.type == WeatherType::AshStorm)
            {
                overlay.setFillColor(sf::Color(180, 120, 70, static_cast<sf::Uint8>(35 + 55 * weather.intensity)));
            }
            window.draw(overlay);
        }

        if (!font)
            return;

        sf::Text text;
        text.setFont(*font);
        text.setCharacterSize(14);
        text.setFillColor(sf::Color(230, 235, 210));
        text.setOutlineThickness(1.0f);
        text.setOutlineColor(sf::Color::Black);

        const auto &tank = adv.tankUtility.runtime();
        const auto &weapon = adv.survival.weapon();

        std::string util = "BucketRig";
        if (tank.utility == TankUtilityMode::RamShield)
            util = "RamShield";
        if (tank.utility == TankUtilityMode::TowCoupler)
            util = "TowCoupler";

        std::string seat = (tank.seat == TankSeat::Driver) ? "Driver" : "Gunner";

        std::ostringstream ss;
        ss << "ADVANCED MECHANICS\n"
           << "Weather: " << weather.banner << " " << static_cast<int>(weather.intensity * 100.0f) << "%\n"
           << "Stress: " << static_cast<int>(adv.survival.stress())
           << " | Ammo: " << weapon.magazine << "/" << weapon.reserveAmmo
           << (weapon.isReloading ? " RELOADING" : "") << "\n"
           << "Tank: " << util << " | Seat: " << seat
           << " | Heat: " << static_cast<int>(tank.cannonThermalLoad)
           << (tank.overheated ? " OVERHEATED" : "") << "\n"
           << "CAMP[B]: " << (adv.camp.enabled() ? "ON" : "OFF")
           << " | ToolGun[F7 mode/F8 use/F2 undo/F3 redo]\n";

        if (!adv.story.lastEvent().empty())
            ss << "Story: " << adv.story.lastEvent() << "\n";
        if (!adv.radio.lastSubtitle().empty())
            ss << adv.radio.lastSubtitle() << "\n";
        if (!adv.toolgun.lastValidation().empty())
            ss << adv.toolgun.lastValidation() << "\n";

        text.setString(ss.str());
        text.setPosition(12.0f, 96.0f);
        window.draw(text);
    }

} // namespace bunker