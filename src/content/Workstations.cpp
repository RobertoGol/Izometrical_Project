#include "content/Workstations.hpp"

namespace bunker
{
    WorkstationDatabase::WorkstationDatabase()
    {
        initialize();
    }

    void WorkstationDatabase::initialize()
    {
        m_Workstations.clear();

        m_Workstations.push_back({3001,
                                  "ARMOR_BENCH_STANDARD",
                                  "Armor Workbench",
                                  WorkstationType::ArmorWorkbench,
                                  {StationActionType::Craft, StationActionType::Repair, StationActionType::Modify},
                                  true,
                                  "Crafting",
                                  12,
                                  4,
                                  0,
                                  4,
                                  0,
                                  false,
                                  0,
                                  1.8f,
                                  1.0f,
                                  1.6f,
                                  "armor_workbench_01",
                                  "ARMOR WORKBENCH",
                                  "Workbench for armor, clothing, and exoskeleton assembly, repair, and modification."});

        m_Workstations.push_back({3002,
                                  "WEAPON_BENCH_STANDARD",
                                  "Weapons Workbench",
                                  WorkstationType::WeaponsWorkbench,
                                  {StationActionType::Craft, StationActionType::Repair, StationActionType::Modify, StationActionType::Scrap},
                                  true,
                                  "Crafting",
                                  10,
                                  4,
                                  0,
                                  3,
                                  1,
                                  false,
                                  0,
                                  1.8f,
                                  1.0f,
                                  1.6f,
                                  "weapons_workbench_01",
                                  "WEAPONS WORKBENCH",
                                  "Workbench for weapon construction, repair, modification, and dismantling."});

        m_Workstations.push_back({3003,
                                  "TINKER_BENCH_STANDARD",
                                  "Tinker Workbench",
                                  WorkstationType::TinkerWorkbench,
                                  {StationActionType::Craft, StationActionType::Scrap},
                                  true,
                                  "Crafting",
                                  8,
                                  3,
                                  2,
                                  4,
                                  2,
                                  false,
                                  0,
                                  1.6f,
                                  1.0f,
                                  1.5f,
                                  "tinker_workbench_01",
                                  "TINKER WORKBENCH",
                                  "Workbench for ammunition, grenades, mines, and utility component assembly."});

        m_Workstations.push_back({3004,
                                  "CHEM_STATION_STANDARD",
                                  "Chem Station",
                                  WorkstationType::ChemStation,
                                  {StationActionType::Craft},
                                  true,
                                  "Crafting",
                                  6,
                                  3,
                                  3,
                                  0,
                                  0,
                                  false,
                                  0,
                                  1.4f,
                                  1.0f,
                                  1.5f,
                                  "chem_station_01",
                                  "CHEM STATION",
                                  "Station for medicine, chemical compounds, and anti-erosion treatment crafting."});

        m_Workstations.push_back({3005,
                                  "TANK_BAY_STANDARD",
                                  "Tank Maintenance Bay",
                                  WorkstationType::TankMaintenanceBay,
                                  {StationActionType::Repair, StationActionType::Modify, StationActionType::Diagnose},
                                  true,
                                  "Maintenance",
                                  40,
                                  10,
                                  12,
                                  10,
                                  8,
                                  true,
                                  6,
                                  3.5f,
                                  3.0f,
                                  2.5f,
                                  "tank_bay_01",
                                  "TANK MAINTENANCE BAY",
                                  "Heavy maintenance platform for BT-7274 diagnostics, repair, and system upgrades."});

        m_Workstations.push_back({3006,
                                  "VAULT_TERMINAL_STANDARD",
                                  "Vault Control Terminal",
                                  WorkstationType::Terminal,
                                  {StationActionType::AccessTerminal, StationActionType::Diagnose},
                                  true,
                                  "Utility",
                                  4,
                                  1,
                                  6,
                                  0,
                                  0,
                                  true,
                                  2,
                                  1.0f,
                                  0.8f,
                                  1.3f,
                                  "vault_terminal_01",
                                  "VAULT TERMINAL",
                                  "Control console for access routing, diagnostics, and bunker administration."});

        m_Workstations.push_back({3007,
                                  "SUPPLY_LOCKER_STANDARD",
                                  "Supply Locker",
                                  WorkstationType::Storage,
                                  {StationActionType::OpenStorage},
                                  true,
                                  "Storage",
                                  6,
                                  4,
                                  0,
                                  0,
                                  0,
                                  false,
                                  0,
                                  1.2f,
                                  1.0f,
                                  1.4f,
                                  "supply_locker_01",
                                  "SUPPLY LOCKER",
                                  "Secure storage locker for components, gear, and field supplies."});
    }

    const WorkstationDef *WorkstationDatabase::findByID(int objectID) const
    {
        for (const auto &ws : m_Workstations)
        {
            if (ws.objectID == objectID)
                return &ws;
        }
        return nullptr;
    }

    const WorkstationDef *WorkstationDatabase::findByType(WorkstationType stationType) const
    {
        for (const auto &ws : m_Workstations)
        {
            if (ws.stationType == stationType)
                return &ws;
        }
        return nullptr;
    }
}