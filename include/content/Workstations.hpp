#include "ui/WorkstationUI.hpp"
#include <iostream>

struct WorkstationUIState
{
    std::string uiTitle = "Workstation";
    std::vector<StationActionType> supportedActions;
    int selectedActionIndex = -1;
    int selectedRecipeIndex = -1;
    std::string statusMessage;
    bool isAvailable = true;
};

namespace bunker
{
    namespace
    {
        // Fallback-заголовок используется только если станция не найдена в
        // WorkstationDatabase (например, WorkstationDef ещё не заведён для
        // этого stationType). UI всё равно должен открыться, а не падать.
        std::string FallbackTitleFor(WorkstationType stationType)
        {
            switch (stationType)
            {
            case WorkstationType::ArmorWorkbench:
                return "ARMOR WORKBENCH";
            case WorkstationType::WeaponsWorkbench:
                return "WEAPONS WORKBENCH";
            case WorkstationType::TinkerWorkbench:
                return "TINKER WORKBENCH";
            case WorkstationType::ChemStation:
                return "CHEM STATION";
            case WorkstationType::TankMaintenanceBay:
                return "TANK MAINTENANCE BAY";
            case WorkstationType::Terminal:
                return "VAULT TERMINAL";
            case WorkstationType::Storage:
                return "SUPPLY LOCKER";
            default:
                return "WORKSTATION";
            }
        }

        std::string ActionName(StationActionType action)
        {
            switch (action)
            {
            case StationActionType::Craft:
                return "Craft";
            case StationActionType::Repair:
                return "Repair";
            case StationActionType::Modify:
                return "Modify";
            case StationActionType::Scrap:
                return "Scrap";
            case StationActionType::Diagnose:
                return "Diagnose";
            case StationActionType::AccessTerminal:
                return "Access Terminal";
            case StationActionType::OpenStorage:
                return "Open Storage";
            default:
                return "Unknown Action";
            }
        }
    }

    WorkstationUIState CreateWorkstationUIState(const WorkstationDatabase &database,
                                                ModularEquipmentSystem &equipmentSystem,
                                                WorkstationType stationType)
    {
        WorkstationUIState state;
        state.stationType = stationType;
        state.visibleRecipes = equipmentSystem.getRecipesForStation(stationType);
        state.isOpen = true;
        state.availability = StationAvailability::Available;

        const WorkstationDef *def = database.findByType(stationType);

        if (def != nullptr)
        {
            state.objectID = def->objectID;
            state.title = !def->uiTitle.empty() ? def->uiTitle : FallbackTitleFor(stationType);
            state.supportedActions = def->supportedActions;
        }
        else
        {
            // Станция не найдена в базе данных — открываем с базовым
            // названием и пустым набором действий вместо падения.
            state.title = FallbackTitleFor(stationType);
            state.supportedActions.clear();
        }

        return state;
    }

    void SelectStationAction(WorkstationUIState &state, int actionIndex)
    {
        if (actionIndex < 0 || actionIndex >= static_cast<int>(state.supportedActions.size()))
        {
            state.selectedActionIndex = -1;
            state.selectedRecipeIndex = -1;
            state.statusMessage = "Invalid action selection.";
            return;
        }

        state.selectedActionIndex = actionIndex;
        state.selectedRecipeIndex = -1; // смена действия сбрасывает выбор рецепта
        state.statusMessage.clear();
    }

    void SelectRecipe(WorkstationUIState &state, int recipeIndex)
    {
        if (recipeIndex < 0 || recipeIndex >= static_cast<int>(state.visibleRecipes.size()))
        {
            state.selectedRecipeIndex = -1;
            state.statusMessage = "Invalid recipe selection.";
            return;
        }

        state.selectedRecipeIndex = recipeIndex;
        state.statusMessage.clear();
    }

    void HandleCraftAction(WorkstationUIState &state, ModularEquipmentSystem &equipmentSystem, PlayerInventory &inventory)
    {
        if (state.selectedRecipeIndex < 0 || state.selectedRecipeIndex >= static_cast<int>(state.visibleRecipes.size()))
        {
            state.statusMessage = "No recipe selected.";
            return;
        }

        const CraftingRecipe &localRecipe = state.visibleRecipes[state.selectedRecipeIndex];

        // visibleRecipes — отфильтрованный список для этой станции, поэтому
        // локальный индекс не совпадает с индексом в общей базе рецептов.
        // Находим реальный индекс в equipmentSystem.recipes() по совпадению
        // resultItemID (это стабильный идентификатор рецепта).
        const auto &allRecipes = equipmentSystem.recipes();
        int realIndex = -1;
        for (int i = 0; i < static_cast<int>(allRecipes.size()); ++i)
        {
            if (allRecipes[i].resultItemID == localRecipe.resultItemID &&
                allRecipes[i].stationType == localRecipe.stationType)
            {
                realIndex = i;
                break;
            }
        }

        if (realIndex < 0)
        {
            state.statusMessage = "Recipe not found in database.";
            return;
        }

        CraftResult result = equipmentSystem.craftItem(inventory, realIndex);

        switch (result)
        {
        case CraftResult::Success:
            state.statusMessage = "Crafted: " + localRecipe.resultName;
            break;
        case CraftResult::InvalidRecipeIndex:
            state.statusMessage = "Craft failed: invalid recipe.";
            break;
        case CraftResult::InsufficientScrap:
            state.statusMessage = "Craft failed: not enough Scrap Metal.";
            break;
        case CraftResult::InsufficientCircuits:
            state.statusMessage = "Craft failed: not enough Circuits.";
            break;
        case CraftResult::InsufficientCoreEnergy:
            state.statusMessage = "Craft failed: not enough Core Energy.";
            break;
        }
    }

    void HandleRepairAction(WorkstationUIState &state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleModifyAction(WorkstationUIState &state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleScrapAction(WorkstationUIState &state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleDiagnoseAction(WorkstationUIState &state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleAccessTerminalAction(WorkstationUIState &state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleOpenStorageAction(WorkstationUIState &state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void ExecuteSelectedAction(WorkstationUIState &state,
                               ModularEquipmentSystem &equipmentSystem,
                               PlayerInventory &inventory)
    {
        if (state.availability != StationAvailability::Available)
        {
            state.statusMessage = "Station is not available right now.";
            return;
        }

        if (state.selectedActionIndex < 0 || state.selectedActionIndex >= static_cast<int>(state.supportedActions.size()))
        {
            state.statusMessage = "No action selected.";
            return;
        }

        StationActionType action = state.supportedActions[state.selectedActionIndex];

        switch (action)
        {
        case StationActionType::Craft:
            HandleCraftAction(state, equipmentSystem, inventory);
            break;
        case StationActionType::Repair:
            HandleRepairAction(state);
            break;
        case StationActionType::Modify:
            HandleModifyAction(state);
            break;
        case StationActionType::Scrap:
            HandleScrapAction(state);
            break;
        case StationActionType::Diagnose:
            HandleDiagnoseAction(state);
            break;
        case StationActionType::AccessTerminal:
            HandleAccessTerminalAction(state);
            break;
        case StationActionType::OpenStorage:
            HandleOpenStorageAction(state);
            break;
        }
    }

    WorkstationUIState OpenWorkstationUI(const WorkstationDatabase &database,
                                         ModularEquipmentSystem &equipmentSystem,
                                         WorkstationType stationType)
    {
        WorkstationUIState state = CreateWorkstationUIState(database, equipmentSystem, stationType);

        std::cout << "=== " << state.title << " ===" << std::endl;

        if (state.supportedActions.empty())
        {
            std::cout << "(no actions available)" << std::endl;
        }
        else
        {
            std::cout << "Actions:" << std::endl;
            for (std::size_t i = 0; i < state.supportedActions.size(); ++i)
            {
                std::cout << "  [" << i << "] " << ActionName(state.supportedActions[i]) << std::endl;
            }
        }

        // Рецепты показываются только если явно выбрано действие Craft.
        if (state.selectedActionIndex >= 0 &&
            state.selectedActionIndex < static_cast<int>(state.supportedActions.size()) &&
            state.supportedActions[state.selectedActionIndex] == StationActionType::Craft)
        {
            std::cout << "Recipes:" << std::endl;
            for (std::size_t i = 0; i < state.visibleRecipes.size(); ++i)
            {
                std::cout << "  [" << i << "] " << state.visibleRecipes[i].resultName << std::endl;
            }
        }

        if (!state.statusMessage.empty())
        {
            std::cout << "Status: " << state.statusMessage << std::endl;
        }

        return state;
    }

    WorkstationUIState OpenWorkstationUIFromWorld(const WorkstationDatabase &database,
                                                  const WorldWorkstation &worldStation,
                                                  ModularEquipmentSystem &equipmentSystem)
    {
        const WorkstationDef *def = database.findByID(worldStation.objectID);

        if (def == nullptr)
        {
            // Объект в мире ссылается на неизвестный WorkstationDef —
            // не падаем, открываем с fallback-заголовком и без действий.
            WorkstationUIState state;
            state.objectID = worldStation.objectID;
            state.title = "UNKNOWN STATION";
            state.isOpen = true;
            state.availability = CheckStationAvailability(worldStation, nullptr);
            state.statusMessage = "This station has no known definition.";
            return state;
        }

        WorkstationUIState state = CreateWorkstationUIState(database, equipmentSystem, def->stationType);
        state.availability = CheckStationAvailability(worldStation, def);

        switch (state.availability)
        {
        case StationAvailability::Destroyed:
            state.statusMessage = "This station is destroyed and cannot be used.";
            break;
        case StationAvailability::Inactive:
            state.statusMessage = "This station is inactive.";
            break;
        case StationAvailability::Unpowered:
            state.statusMessage = "This station has no power.";
            break;
        case StationAvailability::Available:
            break;
        }

        return state;
    }
}