#pragma once

#include "content/Workstations.hpp"
#include "persistence/ModularEquipmentSystem.hpp"

namespace bunker
{
    void OpenWorkstationUI(ModularEquipmentSystem &equipmentSystem,
                           WorkstationType stationType);

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
