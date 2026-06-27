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

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace bunker {

// ═══════════════════════════════════════════════════════
// Отрисовка изометрического пола
// ═══════════════════════════════════════════════════════
void GameRenderer::renderFloor(sf::RenderWindow& window,
                               const GameState& gs,
                               const TimeShift& timeShift) {
    for (int x = 0; x < Config::MAP_WIDTH; ++x) {
        for (int y = 0; y < Config::MAP_HEIGHT; ++y) {
            sf::ConvexShape tile(4);
            tile.setPoint(0, IsoMath::worldToScreen(static_cast<float>(x),     static_cast<float>(y)));
            tile.setPoint(1, IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y)));
            tile.setPoint(2, IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y + 1)));
            tile.setPoint(3, IsoMath::worldToScreen(static_cast<float>(x),     static_cast<float>(y + 1)));

            if (gs.sectorMap[x][y] == 1) {
                if (timeShift.isPast()) {
                    tile.setFillColor(sf::Color(60, 70, 90));       // Прошлое — синеватые стены
                    tile.setOutlineColor(sf::Color(80, 90, 115));
                } else {
                    tile.setFillColor(sf::Color(70, 75, 85));       // Настоящее — серые стены
                    tile.setOutlineColor(sf::Color(90, 95, 105));
                }
            } else if (gs.etherErosionMap[x][y] > 5.0f) {
                int intensity = static_cast<int>(std::min(gs.etherErosionMap[x][y] * 1.5f, 80.0f));
                tile.setFillColor(sf::Color(30 + intensity / 2, 20, 35 + intensity));
                tile.setOutlineColor(sf::Color(50, 30, 60));
            } else {
                if (timeShift.isPast()) {
                    tile.setFillColor(sf::Color(30, 35, 45));       // Прошлое — холодные тона
                    tile.setOutlineColor(sf::Color(45, 50, 60));
                } else {
                    tile.setFillColor(sf::Color(35, 35, 40));       // Настоящее
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
void GameRenderer::renderEntities(sf::RenderWindow& window,
                                  const GameState& gs,
                                  const TimeShift& timeShift,
                                  const HostileAISystem& hostileAI) {
    std::vector<RenderObject> renderQueue;

    // Pip-Pad на полу
    if (!gs.bunkerProgression.hasFoundPipPad) {
        float px = gs.bunkerProgression.pipPadSpawnPos.x;
        float py = gs.bunkerProgression.pipPadSpawnPos.y;
        renderQueue.push_back({ px + py, [&, px, py]() {
            sf::RectangleShape pad({10, 8});
            pad.setFillColor(sf::Color(255, 255, 50, 200));
            pad.setOrigin(5, 4);
            pad.setPosition(IsoMath::worldToScreen(px, py));
            window.draw(pad);
        }});
    }

    // Лут-контейнеры
    for (const auto& c : gs.lootContainers) {
        float cx = c.position.x;
        float cy = c.position.y;
        bool opened = c.isOpened;
        renderQueue.push_back({ cx + cy, [&, cx, cy, opened]() {
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
    const auto& hostileStates = hostileAI.debugStates();
    for (std::size_t i = 0; i < gs.enemies.size(); ++i) {
        const auto& e = gs.enemies[i];
        if (!e.isAlive) continue;

        float ex = e.position.x;
        float ey = e.position.y;
        float er = e.radius;
        HostileKind kind = HostileKind::VerminRush;
        HostileAlertState alert = HostileAlertState::Idle;
        if (i < hostileStates.size()) {
            kind = hostileStates[i].kind;
            alert = hostileStates[i].alert;
        }

        renderQueue.push_back({ ex + ey, [&, ex, ey, er, isPast, kind, alert]() {
            sf::Color body = sf::Color(255, 50, 50);
            int points = 3;

            switch (kind) {
                case HostileKind::VerminRush:
                    body = sf::Color(210, 45, 45);
                    points = 3;
                    break;
                case HostileKind::GhoulRush:
                    body = sf::Color(150, 230, 80);
                    points = 8;
                    break;
                case HostileKind::HumanTactical:
                    body = sf::Color(235, 170, 70);
                    points = 4;
                    break;
                case HostileKind::RobotControl:
                    body = sf::Color(135, 170, 230);
                    points = 6;
                    break;
            }

            if (isPast) {
                body = sf::Color(
                    static_cast<sf::Uint8>(std::min(255, body.r / 2 + 50)),
                    static_cast<sf::Uint8>(std::min(255, body.g / 2 + 90)),
                    static_cast<sf::Uint8>(std::min(255, body.b + 40)),
                    220
                );
            }

            float radius = 8.0f + er * 5.0f;
            sf::CircleShape shape(radius, points);
            shape.setFillColor(body);
            shape.setOrigin(radius, radius);
            shape.setPosition(IsoMath::worldToScreen(ex, ey));

            if (alert == HostileAlertState::Aggro) {
                shape.setOutlineThickness(2.0f);
                shape.setOutlineColor(sf::Color(255, 40, 40));
            } else if (alert == HostileAlertState::Suspicious ||
                       alert == HostileAlertState::Searching) {
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
        renderQueue.push_back({ tx + ty, [&, tx, ty]() {
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
        renderQueue.push_back({ bx + by, [&, bx, by, piloted]() {
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
    if (gs.playerMode != UnitMode::Titan) {
        float ppx = gs.playerPos.x;
        float ppy = gs.playerPos.y;
        renderQueue.push_back({ ppx + ppy + 0.01f, [&, ppx, ppy]() {
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
        [](const RenderObject& a, const RenderObject& b) {
            return a.depth < b.depth;
        });

    for (auto& obj : renderQueue) {
        obj.drawFunc();
    }
}

// ═══════════════════════════════════════════════════════
// Отрисовка перенесённых advanced-механик:
// Weather, C.A.M.P., breakables, shock waves.
// ═══════════════════════════════════════════════════════
void GameRenderer::renderAdvancedWorld(sf::RenderWindow& window,
                                       const AdvancedMechanics& adv) {
    // Разрушаемые объекты
    for (const auto& b : adv.reactive.breakables()) {
        if (b.broken) continue;

        sf::Color color = sf::Color(150, 110, 60);
        int points = 4;
        float radius = 8.0f;
        switch (b.kind) {
            case BreakableKind::Glass:      color = sf::Color(125, 210, 255, 150); points = 4;  radius = 7.0f;  break;
            case BreakableKind::Vegetation: color = sf::Color(75, 170, 70);        points = 7;  radius = 9.0f;  break;
            case BreakableKind::Crate:      color = sf::Color(170, 120, 55);       points = 4;  radius = 9.0f;  break;
            case BreakableKind::Barrel:     color = sf::Color(210, 75, 45);        points = 12; radius = 8.0f;  break;
            case BreakableKind::Console:    color = sf::Color(85, 165, 210);       points = 6;  radius = 10.0f; break;
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
    for (const auto& w : adv.reactive.waves()) {
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
    for (const auto& obj : adv.camp.objects()) {
        sf::Vector2f pos = IsoMath::worldToScreen(obj.tileX + 0.5f, obj.tileY + 0.5f);
        if (obj.type == CampObjectType::DefenseTurret) {
            sf::CircleShape t(9.0f, 3);
            t.setOrigin(9.0f, 9.0f);
            t.setPosition(pos);
            t.setFillColor(sf::Color(210, 210, 70));
            t.setOutlineThickness(1.5f);
            t.setOutlineColor(sf::Color::Black);
            window.draw(t);
        } else if (obj.type == CampObjectType::SupplyCrate) {
            sf::RectangleShape c({14.0f, 12.0f});
            c.setOrigin(7.0f, 6.0f);
            c.setPosition(pos);
            c.setFillColor(sf::Color(80, 200, 110));
            c.setOutlineThickness(1.0f);
            c.setOutlineColor(sf::Color::Black);
            window.draw(c);
        }
    }

    // Превью строительства
    if (adv.camp.enabled()) {
        const auto& p = adv.camp.preview();
        sf::ConvexShape tile(4);
        tile.setPoint(0, IsoMath::worldToScreen(static_cast<float>(p.tileX),     static_cast<float>(p.tileY)));
        tile.setPoint(1, IsoMath::worldToScreen(static_cast<float>(p.tileX + 1), static_cast<float>(p.tileY)));
        tile.setPoint(2, IsoMath::worldToScreen(static_cast<float>(p.tileX + 1), static_cast<float>(p.tileY + 1)));
        tile.setPoint(3, IsoMath::worldToScreen(static_cast<float>(p.tileX),     static_cast<float>(p.tileY + 1)));
        tile.setFillColor(p.isPlacementValid ? sf::Color(60, 220, 90, 75) : sf::Color(220, 60, 60, 75));
        tile.setOutlineThickness(2.0f);
        tile.setOutlineColor(p.isPlacementValid ? sf::Color(90, 255, 120) : sf::Color(255, 90, 90));
        window.draw(tile);
    }
}

void GameRenderer::renderAdvancedHUD(sf::RenderWindow& window,
                                     const AdvancedMechanics& adv,
                                     const sf::Font* font) {
    // Погодный экранный фильтр
    const auto& weather = adv.weather.state();
    if (weather.type != WeatherType::Clear) {
        sf::RectangleShape overlay({static_cast<float>(Config::SCREEN_WIDTH), static_cast<float>(Config::SCREEN_HEIGHT)});
        if (weather.type == WeatherType::EtherFog) {
            overlay.setFillColor(sf::Color(80, 120, 180, static_cast<sf::Uint8>(45 + 60 * weather.intensity)));
        }
        if (weather.type == WeatherType::AcidRain) {
            overlay.setFillColor(sf::Color(70, 180, 70, static_cast<sf::Uint8>(35 + 55 * weather.intensity)));
        }
        if (weather.type == WeatherType::AshStorm) {
            overlay.setFillColor(sf::Color(180, 120, 70, static_cast<sf::Uint8>(35 + 55 * weather.intensity)));
        }
        window.draw(overlay);
    }

    if (!font) return;

    sf::Text text;
    text.setFont(*font);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color(230, 235, 210));
    text.setOutlineThickness(1.0f);
    text.setOutlineColor(sf::Color::Black);

    const auto& tank = adv.tankUtility.runtime();
    const auto& weapon = adv.survival.weapon();

    std::string util = "BucketRig";
    if (tank.utility == TankUtilityMode::RamShield) util = "RamShield";
    if (tank.utility == TankUtilityMode::TowCoupler) util = "TowCoupler";

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

    if (!adv.story.lastEvent().empty()) ss << "Story: " << adv.story.lastEvent() << "\n";
    if (!adv.radio.lastSubtitle().empty()) ss << adv.radio.lastSubtitle() << "\n";
    if (!adv.toolgun.lastValidation().empty()) ss << adv.toolgun.lastValidation() << "\n";

    text.setString(ss.str());
    text.setPosition(12.0f, 96.0f);
    window.draw(text);
}

}  // namespace bunker
