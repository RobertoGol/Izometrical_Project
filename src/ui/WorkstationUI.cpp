#include "ui/WorkstationUI.hpp"
#include <iostream>

namespace bunker
{
    void OpenWorkstationUI(ModularEquipmentSystem &equipmentSystem,
                           WorkstationType stationType)
    {
        auto recipes = equipmentSystem.getRecipesForStation(stationType);

        std::cout << "=== WORKSTATION OPENED ===" << std::endl;

        for (const auto &recipe : recipes)
        {
            std::cout << recipe.resultName << std::endl;
        }
    }
}