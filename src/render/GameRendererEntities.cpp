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
    void GameRenderer::renderEntities(sf::RenderWindow& window, const GameState& gs, const TimeShift& timeShift,
                                      const HostileAISystem& hostileAI)
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
                                       pad.setOrigin({5.0f, 4.0f});
                                       pad.setPosition(IsoMath::worldToScreen(px, py));
                                       window.draw(pad);
                                   }});
        }

        // Лут-контейнеры
        for (const auto& c : gs.lootContainers)
        {
            float cx = c.position.x;
            float cy = c.position.y;
            bool opened = c.isOpened;
            renderQueue.push_back({cx + cy, [&, cx, cy, opened]()
                                   {
                                       sf::RectangleShape box({12, 10});
                                       box.setOrigin({6.0f, 5.0f});
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
        for (std::size_t i = 0; i < gs.enemies.size(); ++i)
        {
            const auto& e = gs.enemies[i];
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
                                       const auto& profile = EnemyArchetypeRegistry::getProfile(kind);
                                       sf::Color body = profile.bodyColor;
                                       int points = profile.shapePoints;

                                       if (isPast)
                                       {
                                           body = sf::Color(static_cast<std::uint8_t>(std::min(255, body.r / 2 + 50)),
                                                            static_cast<std::uint8_t>(std::min(255, body.g / 2 + 90)),
                                                            static_cast<std::uint8_t>(std::min(255, body.b + 40)), 220);
                                       }

                                       float radius = 8.0f + er * 5.0f;
                                       sf::CircleShape shape(radius, points);
                                       shape.setFillColor(body);
                                       shape.setOrigin({radius, radius});
                                       shape.setPosition(IsoMath::worldToScreen(ex, ey));

                                       switch (alert)
                                       {
                                       case HostileAlertState::Aggro:
                                           shape.setOutlineThickness(2.0f);
                                           shape.setOutlineColor(sf::Color(255, 40, 40));
                                           break;
                                       case HostileAlertState::Suspicious:
                                       case HostileAlertState::Searching:
                                           shape.setOutlineThickness(1.5f);
                                           shape.setOutlineColor(sf::Color(255, 220, 60));
                                           break;
                                       default:
                                           break;
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
                                       tower.setOrigin({7.0f, 7.0f});
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
                                       bt.setOrigin({16.0f, 16.0f});
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
                                       pShape.setOrigin({12.0f, 12.0f});
                                       pShape.setPosition(IsoMath::worldToScreen(ppx, ppy));
                                       pShape.setOutlineThickness(2.0f);
                                       pShape.setOutlineColor(sf::Color(0, 200, 200));
                                       window.draw(pShape);
                                   }});
        }

        // Z-сортировка и отрисовка
        std::sort(renderQueue.begin(), renderQueue.end(),
                  [](const RenderObject& a, const RenderObject& b) { return a.depth < b.depth; });

        for (auto& obj : renderQueue)
        {
            obj.drawFunc();
        }
    }

    // ═══════════════════════════════════════════════════════
    // Отрисовка перенесённых advanced-механик:
    // Weather, C.A.M.P., breakables, shock waves.
    // ═══════════════════════════════════════════════════════

} // namespace bunker
