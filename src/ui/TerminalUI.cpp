#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "persistence/Inventory.hpp"
#include "persistence/Progression.hpp"
#include "core/Constants.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace bunker
{

    struct BunkerTerminal
    {
        int id = 0;
        Vector3D position;
        std::string title;
        std::string logContent;
        int clearanceTier = 1;
        bool isSynced = false;
    };

    class TerminalManager
    {
    private:
        std::vector<BunkerTerminal> m_Terminals;
        int m_ActiveTerminalIndex = -1;
        bool m_IsOpen = false;
        float m_CrtScanlineTimer = 0.0f;
        sf::Font m_Font;
        bool m_FontLoaded = false;

    public:
        TerminalManager();

        void loadFont(const sf::Font &font)
        {
            m_Font = font;
            m_FontLoaded = true;
        }
        void spawnDefaultTerminals();
        bool tryInteractTerminal(GameState &gs);
        void close()
        {
            m_IsOpen = false;
            m_ActiveTerminalIndex = -1;
        }
        void handleWindowEvents(sf::RenderWindow &window, GameState &gs);
        void update(float dt);
        void render(sf::RenderWindow &window, const GameState &gs) const;

        bool isOpen() const { return m_IsOpen; }
        const BunkerTerminal *getNearestTerminal(const Vector3D &playerPos, float maxRange = 1.8f) const;
        const std::vector<BunkerTerminal> &getTerminals() const { return m_Terminals; }

    private:
        void drawText(sf::RenderWindow &window, const std::string &str, float x, float y, int size, sf::Color color) const;
    };

} // namespace bunker
