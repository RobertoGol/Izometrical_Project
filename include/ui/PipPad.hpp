#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Inventory.hpp"
#include "gameplay/AdvancedMechanics.hpp"
#include <SFML/Graphics.hpp>
#include <string>

namespace bunker
{

    enum class PipBoyVersionStyle
    {
        Mark1_PaperMap,     // Версия 1: Бумажная чертёжная топография секторов
        Mark2_ElectronicCRT // Версия 2: Электронный зелёно-янтарный векторный CRT радар
    };

    class PipPadUI
    {
    public:
        PipPadUI() = default;

        void handleInput(const sf::Event &event, GameState &gs, PlayerInventory &inv, AdvancedMechanics &adv);
        void update(GameState &gs, float dt);
        void renderFlashlight(sf::RenderWindow &window, const GameState &gs) const;
        void renderTablet(sf::RenderWindow &window, const GameState &gs, const PlayerInventory &inv, const AdvancedMechanics &adv) const;

        bool isFlashlightActive() const { return m_FlashlightActive; }
        bool isTabletOpen() const { return m_TabletOpen; }

    private:
        bool m_FlashlightActive = false;
        bool m_TabletOpen = false;
        int m_ActiveTab = 0; // 0=Inventory, 1=Map (Paper/CRT), 2=Tapes
        float m_BatteryLevel = 100.0f;
    };

} // namespace bunker
