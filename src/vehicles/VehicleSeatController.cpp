#include "vehicles/VehicleSeatController.hpp"

namespace bunker
{

    VehicleSeatController::VehicleSeatController()
    {
        m_Seats[0] = {false, CockpitRole::Driver};
        m_Seats[1] = {false, CockpitRole::Gunner};
    }

    bool VehicleSeatController::tryMountPlayer(GameState &gs, CockpitRole role)
    {
        int idx = (role == CockpitRole::Driver) ? 0 : 1;
        if (m_Seats[idx].isOccupied)
            return false;

        m_Seats[idx].isOccupied = true;
        m_ActivePlayerRole = role;
        gs.playerMode = UnitMode::Titan;
        gs.titan.isPiloted = true;

        std::cout << "[COCKPIT] Пилот занял место: "
                  << ((role == CockpitRole::Driver) ? "ВОДИТЕЛЬ (Орудие XO-16)" : "СТРЕЛОК (Пулемёты турели)")
                  << std::endl;
        return true;
    }

    void VehicleSeatController::dismountPlayer(GameState &gs)
    {
        m_Seats[0].isOccupied = false;
        m_Seats[1].isOccupied = false;
        gs.playerMode = UnitMode::Scout;
        gs.titan.isPiloted = false;
        std::cout << "[COCKPIT] Пилот спешился из кабины БТ-7274." << std::endl;
    }

    void VehicleSeatController::swapPlayerRole()
    {
        if (m_ActivePlayerRole == CockpitRole::Driver)
        {
            m_Seats[0].isOccupied = false;
            m_Seats[1].isOccupied = true;
            m_ActivePlayerRole = CockpitRole::Gunner;
            std::cout << "[SEAT SWAP] Пересадка на место СТРЕЛКА." << std::endl;
        }
        else
        {
            m_Seats[1].isOccupied = false;
            m_Seats[0].isOccupied = true;
            m_ActivePlayerRole = CockpitRole::Driver;
            std::cout << "[SEAT SWAP] Пересадка на место ВОДИТЕЛЯ." << std::endl;
        }
    }

} // namespace bunker