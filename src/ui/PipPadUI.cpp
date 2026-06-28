#include "ui/PipPadUI.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace bunker
{

    void PipPadUI::handleInput(const sf::Event &event, GameState &gs, PlayerInventory &inv, AdvancedMechanics &adv)
    {
        (void)inv;
        (void)adv;
        if (!gs.bunkerProgression.hasFoundPipPad)
            return;

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::I)
            {
                m_TabletOpen = !m_TabletOpen;
                m_ActiveTab = 0;
                if (m_TabletOpen)
                    std::cout << "[PIP-BOY] Вкладка INVENTORY (Снаряжение на руке)." << std::endl;
            }
            else if (event.key.code == sf::Keyboard::M)
            {
                m_TabletOpen = !m_TabletOpen;
                m_ActiveTab = 1;
                if (m_TabletOpen)
                    std::cout << "[PIP-BOY] Вкладка MAP (Топография Убежища 17)." << std::endl;
            }
            else if (event.key.code == sf::Keyboard::Escape && m_TabletOpen)
            {
                m_TabletOpen = false;
            }
        }
    }

    void PipPadUI::update(GameState &gs, float dt)
    {
        if (!gs.bunkerProgression.hasFoundPipPad)
        {
            m_FlashlightActive = false;
            m_TabletOpen = false;
            return;
        }

        bool holdTab = sf::Keyboard::isKeyPressed(sf::Keyboard::Tab);
        if (holdTab && !m_FlashlightActive && m_BatteryLevel > 0.0f)
        {
            m_FlashlightActive = true;
        }
        else if (!holdTab && m_FlashlightActive)
        {
            m_FlashlightActive = false;
        }

        if (m_FlashlightActive)
        {
            m_BatteryLevel = std::max(0.0f, m_BatteryLevel - 2.5f * dt);
            if (m_BatteryLevel <= 0.0f)
                m_FlashlightActive = false;
        }
        else
        {
            m_BatteryLevel = std::min(100.0f, m_BatteryLevel + 5.0f * dt);
        }
    }

    void PipPadUI::renderFlashlight(sf::RenderWindow &window, const GameState &gs) const
    {
        if (!m_FlashlightActive || !gs.bunkerProgression.hasFoundPipPad)
            return;

        const float cellW = gs.windowWidth / static_cast<float>(Config::MAP_WIDTH);
        const float cellH = gs.windowHeight / static_cast<float>(Config::MAP_HEIGHT);
        const float playerScreenX = gs.playerPos.x * cellW;
        const float playerScreenY = gs.playerPos.y * cellH;

        sf::CircleShape lightBeam(cellW * 4.5f);
        lightBeam.setOrigin(cellW * 4.5f, cellH * 4.5f);
        lightBeam.setPosition(playerScreenX, playerScreenY);

        sf::Uint8 alpha = static_cast<sf::Uint8>(std::clamp(m_BatteryLevel * 0.45f + 15.0f, 20.0f, 65.0f));
        lightBeam.setFillColor(sf::Color(180, 255, 190, alpha));
        window.draw(lightBeam);
    }

    void PipPadUI::renderTablet(sf::RenderWindow &window, const GameState &gs, const PlayerInventory &inv, const AdvancedMechanics &adv) const
    {
        (void)inv;
        (void)adv;
        if (!m_TabletOpen || !gs.bunkerProgression.hasFoundPipPad)
            return;

        // Компактная проекция планшета Pip-Boy на запястье (Не заслоняет весь экран):
        const float W = gs.windowWidth;
        const float H = gs.windowHeight;
        sf::RectangleShape tabletBg({W - 200.0f, H - 180.0f});
        tabletBg.setPosition(100.0f, 90.0f);

        bool isPaperMapStyle = (gs.characterProg.level <= 1);
        if (m_ActiveTab == 1 && isPaperMapStyle)
        {
            // Бумажная карта (Марк 1):
            tabletBg.setFillColor(sf::Color(215, 202, 168));
            tabletBg.setOutlineThickness(3.0f);
            tabletBg.setOutlineColor(sf::Color(110, 95, 65));
        }
        else
        {
            // Электронный векторный CRT терминал (Марк 2):
            tabletBg.setFillColor(sf::Color(10, 24, 12, 240));
            tabletBg.setOutlineThickness(3.0f);
            tabletBg.setOutlineColor(sf::Color(45, 235, 80));
        }
        window.draw(tabletBg);
    }

} // namespace bunker
