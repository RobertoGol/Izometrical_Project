#pragma once

#include "content/Workstations.hpp"
#include "persistence/ModularEquipmentSystem.hpp"
#include <string>
#include <vector>

namespace bunker
{
    struct WorkstationUIState
    {
        WorkstationType stationType = WorkstationType::ArmorWorkbench;
        std::string title;
        std::vector<CraftingRecipe> visibleRecipes;
        int selectedRecipeIndex = -1;
        bool isOpen = false;
    };

    WorkstationUIState CreateWorkstationUIState(ModularEquipmentSystem &equipmentSystem,
                                                WorkstationType stationType);

    void OpenWorkstationUI(ModularEquipmentSystem &equipmentSystem,
                           WorkstationType stationType);
}