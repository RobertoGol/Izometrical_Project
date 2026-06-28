#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Inventory.hpp"
#include "gameplay/AdvancedMechanics.hpp"
#include <SFML/Graphics.hpp>
#include <string>

namespace bunker
{

    class PipPadUI
    {
    public:
        PipPadUI() = default;

        void toggleTab(GameState &gs, int tabIndex);
        void update(GameState &gs, float dt);
        void renderFlashlight(sf::RenderWindow &window, const GameState &gs) const;
        void renderTablet(sf::RenderWindow &window, const GameState &gs, const PlayerInventory &inv, const AdvancedMechanics &adv, const sf::Font *font) const;

        bool isFlashlightActive() const { return m_FlashlightActive; }
        bool isTabletOpen() const { return m_TabletOpen; }

    private:
        bool m_FlashlightActive = false;
        bool m_TabletOpen = false;
        int m_ActiveTab = 0; // 0=Inventory, 1=Map (Paper/CRT)
        float m_BatteryLevel = 100.0f;
    };

} // namespace bunker
