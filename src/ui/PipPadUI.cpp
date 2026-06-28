#include "ui/PipPad.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace bunker
{

    void PipPadUI::toggleTab(GameState &gs, int tabIndex)
    {
        if (!gs.bunkerProgression.hasFoundPipPad)
        {
            std::cout << "[PIP-BOY] Наручный гаджет Пилота не найден на полу бункера!" << std::endl;
            return;
        }

        if (!m_TabletOpen || m_ActiveTab != tabIndex)
        {
            m_TabletOpen = true;
            m_ActiveTab = tabIndex;
            std::cout << "[PIP-BOY] Активирован наручный планшет (Вкладка: "
                      << (tabIndex == 0 ? "TACTICAL INVENTORY & BUFFS" : "VAULT 17 MAP ARCHIVE") << ")." << std::endl;
        }
        else
        {
            m_TabletOpen = false;
            std::cout << "[PIP-BOY] Планшет сложен (Возврат в тактический обзор арены без миникарты)." << std::endl;
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

    void PipPadUI::renderTablet(sf::RenderWindow &window, const GameState &gs, const PlayerInventory &inv, const AdvancedMechanics &adv, const sf::Font *font) const
    {
        (void)inv;
        (void)adv;
        if (!m_TabletOpen || !gs.bunkerProgression.hasFoundPipPad)
            return;

        const float W = gs.windowWidth;
        const float H = gs.windowHeight;
        sf::RectangleShape tabletBg({W - 180.0f, H - 160.0f});
        tabletBg.setPosition(90.0f, 80.0f);

        bool isPaperMapStyle = (gs.characterProg.level <= 1);
        sf::Color textColor = sf::Color(50, 240, 90);

        if (m_ActiveTab == 1 && isPaperMapStyle)
        {
            tabletBg.setFillColor(sf::Color(215, 202, 168));
            tabletBg.setOutlineThickness(3.0f);
            tabletBg.setOutlineColor(sf::Color(110, 95, 65));
            textColor = sf::Color(45, 35, 25);
        }
        else
        {
            tabletBg.setFillColor(sf::Color(10, 24, 12, 240));
            tabletBg.setOutlineThickness(3.0f);
            tabletBg.setOutlineColor(sf::Color(45, 235, 80));
        }
        window.draw(tabletBg);

        if (!font)
            return;

        auto drawTxt = [&](const std::string &s, float x, float y, int sz, sf::Color c)
        {
            sf::Text t;
            t.setFont(*font);
            t.setString(s);
            t.setCharacterSize(sz);
            t.setFillColor(c);
            t.setPosition(x, y);
            window.draw(t);
        };

        float sx = 120.0f, sy = 110.0f;

        if (m_ActiveTab == 0)
        {
            drawTxt("PIP-BOY WEARABLE — TACTICAL INVENTORY (V17)", sx, sy, 16, sf::Color(50, 255, 100));
            drawTxt("===========================================", sx, sy + 22, 16, sf::Color(30, 180, 70));

            int row = 0;
            for (const auto &slot : inv.getSlots())
            {
                std::string l = "> [" + std::to_string(slot.quantity) + "x] " + slot.displayName + " (wt: " + std::to_string(slot.weightPerUnit) + ")";
                if (slot.itemID == 520)
                    l += " [JAKE RAYMOR ART: +35% SPEED NUKASHINE BUFF]";
                if (slot.itemID == 521)
                    l += " [JAKE RAYMOR ART: WEIGHT BE-GONE POTION]";
                if (slot.itemID == 710)
                    l += " [JAKE RAYMOR ART: SHEEPSQUATCH CLUB]";
                if (slot.itemID == 810)
                    l += " [JAKE RAYMOR ART: SPACE EXPLORER BACKPACK]";

                drawTxt(l, sx, sy + 60 + row * 24, 14, sf::Color(180, 255, 190));
                row++;
            }
            drawTxt("[I] Сложить планшет на запястье | На основном экране миникарты нет", sx, H - 120.0f, 13, sf::Color(120, 200, 130));
        }
        else if (m_ActiveTab == 1)
        {
            std::string title = isPaperMapStyle
                                    ? "VAULT 17 — TOPOGRAPHIC PAPER MAP (PIP-BOY MARK I)"
                                    : "VAULT 17 — ELECTRONIC VECTOR CRT RADAR (PIP-BOY MARK II)";
            drawTxt(title, sx, sy, 16, textColor);
            drawTxt("==================================================", sx, sy + 22, 16, textColor);

            float gw = W - 260.0f, gh = H - 280.0f;
            float cw = gw / static_cast<float>(Config::MAP_WIDTH), ch = gh / static_cast<float>(Config::MAP_HEIGHT);
            for (int x = 0; x < Config::MAP_WIDTH; ++x)
            {
                for (int y = 0; y < Config::MAP_HEIGHT; ++y)
                {
                    if (gs.sectorMap[x][y] == 1)
                    {
                        sf::RectangleShape w({cw - 1.0f, ch - 1.0f});
                        w.setPosition(sx + x * cw, sy + 60 + y * ch);
                        w.setFillColor(isPaperMapStyle ? sf::Color(55, 42, 28) : sf::Color(35, 210, 70));
                        window.draw(w);
                    }
                }
            }
            sf::CircleShape pm(cw * 0.4f);
            pm.setPosition(sx + gs.playerPos.x * cw - cw * 0.4f, sy + 60 + gs.playerPos.y * ch - ch * 0.4f);
            pm.setFillColor(isPaperMapStyle ? sf::Color(200, 30, 30) : sf::Color(100, 255, 255));
            window.draw(pm);

            drawTxt("[M] Сложить карту прибора в тактический походный режим", sx, H - 120.0f, 13, textColor);
        }
    }

} // namespace bunker
