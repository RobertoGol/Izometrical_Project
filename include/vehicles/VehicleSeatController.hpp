#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include <array>
#include <iostream>

namespace bunker
{

    enum class CockpitRole
    {
        Driver, // Место 0: Ходовая часть и пушка XO-16
        Gunner  // Место 1: Турельные пулемёты
    };

    struct SeatSlot
    {
        bool isOccupied = false;
        CockpitRole role = CockpitRole::Driver;
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Контроллер мест в технике (Волна 5 / Seat Swap)
    // Управляет пересадкой Водитель ↔ Стрелок без нарушения пешего режима
    // ═══════════════════════════════════════════════════════════════════════

    class VehicleSeatController
    {
    private:
        std::array<SeatSlot, 2> m_Seats;
        CockpitRole m_ActivePlayerRole = CockpitRole::Driver;

    public:
        VehicleSeatController();

        bool tryMountPlayer(GameState &gs, CockpitRole role);
        void dismountPlayer(GameState &gs);
        void swapPlayerRole();

        CockpitRole activeRole() const { return m_ActivePlayerRole; }
        bool isDriverOccupied() const { return m_Seats[0].isOccupied; }
        bool isGunnerOccupied() const { return m_Seats[1].isOccupied; }
    };

} // namespace bunker