#pragma once

#include <SFML/Graphics.hpp>
#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "entities/PlayerController.hpp"
#include "gameplay/Tactics.hpp"
#include "ai/TitanAI.hpp"
#include "vehicles/VehicleManager.hpp"
#include "persistence/Inventory.hpp"
#include "core/Constants.hpp"
#include <string>

namespace bunker
{

    class HUD
    {
    private:
        sf::Font m_Font;
        bool m_FontLoaded = false;

        void drawBar(sf::RenderWindow &window, float x, float y, float width, float height,
                     float percent, sf::Color bgColor, sf::Color fillColor,
                     sf::Color borderColor = sf::Color::Transparent);

        void drawText(sf::RenderWindow &window, const std::string &str,
                      float x, float y, int size, sf::Color color = sf::Color::White);

        void drawTankFrame(sf::RenderWindow &window, float W, float H);

        void renderScoutHUD(sf::RenderWindow &window, const GameState &gs,
                            const PlayerController &ctrl, const TacticsManager &tactics,
                            const PlayerInventory &inventory, float W, float H);

        void renderTankHUD(sf::RenderWindow &window, const GameState &gs,
                           const TitanAI &titanAI, float W, float H);

        void renderVehicleHUD(sf::RenderWindow &window, const GameState &gs,
                              const VehicleManager &vehicles, float W, float H);

        void renderScore(sf::RenderWindow &window, const GameState &gs, float W);
        void renderErosionWarning(sf::RenderWindow &window, const GameState &gs, float W, float H);
        void renderPipPadHint(sf::RenderWindow &window, const GameState &gs, float W, float H);

    public:
        HUD() = default;

        void loadFont(const std::string &fontPath = "assets/fonts/default.ttf")
        {
            if (m_Font.loadFromFile(fontPath))
            {
                m_FontLoaded = true;
            }
        }

        void render(sf::RenderWindow &window, const GameState &gs,
                    const PlayerController &playerCtrl,
                    const TacticsManager &tactics,
                    const TitanAI &titanAI,
                    const VehicleManager &vehicles,
                    const PlayerInventory &inventory);
    };

} // namespace bunker