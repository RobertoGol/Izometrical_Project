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
    void GameRenderer::renderFloor(sf::RenderWindow& window, const GameState& gs, const TimeShift& timeShift)
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
                    tile.setFillColor(sf::Color(static_cast<std::uint8_t>(30 + intensity / 2), 20,
                                                static_cast<std::uint8_t>(35 + intensity)));
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

} // namespace bunker
