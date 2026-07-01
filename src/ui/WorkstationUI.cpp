#include "ui/WorkstationUI.hpp"
#include <iostream>

namespace bunker
{
    WorkstationUIState CreateWorkstationUIState(ModularEquipmentSystem &equipmentSystem,
                                                WorkstationType stationType)
    {
        WorkstationUIState state;
        state.stationType = stationType;
        state.visibleRecipes = equipmentSystem.getRecipesForStation(stationType);
        state.isOpen = true;

        switch (stationType)
        {
        case WorkstationType::ArmorWorkbench:
            state.title = "ARMOR WORKBENCH";
            break;
        case WorkstationType::WeaponsWorkbench:
            state.title = "WEAPONS WORKBENCH";
            break;
        case WorkstationType::TinkerWorkbench:
            state.title = "TINKER WORKBENCH";
            break;
        case WorkstationType::ChemStation:
            state.title = "CHEM STATION";
            break;
        case WorkstationType::TankMaintenanceBay:
            state.title = "TANK MAINTENANCE BAY";
            break;
        case WorkstationType::Terminal:
            state.title = "VAULT TERMINAL";
            break;
        case WorkstationType::Storage:
            state.title = "SUPPLY LOCKER";
            break;
        default:
            state.title = "WORKSTATION";
            break;
        }

        return state;
    }

    void OpenWorkstationUI(ModularEquipmentSystem &equipmentSystem,
                           WorkstationType stationType)
    {
        auto state = CreateWorkstationUIState(equipmentSystem, stationType);

        std::cout << "=== " << state.title << " ===" << std::endl;

        for (const auto &recipe : state.visibleRecipes)
        {
            std::cout << recipe.resultName << std::endl;
        }
    }
}