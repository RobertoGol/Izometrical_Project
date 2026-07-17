#include "ui/WorkstationUI.hpp"

#include "engine/Log.hpp"

namespace bunker
{
    namespace
    {
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

        StationAvailability CheckStationAvailability(const WorldWorkstation& worldStation, const WorkstationDef* def)
        {
            if (worldStation.isDestroyed)
            {
                return StationAvailability::Destroyed;
            }
            if (!worldStation.isActive)
            {
                return StationAvailability::Inactive;
            }
            if ((def != nullptr && def->requiresPower) && !worldStation.isPowered)
            {
                return StationAvailability::Unpowered;
            }
            return StationAvailability::Available;
        }
    } // namespace

    WorkstationUIState CreateWorkstationUIState(const WorkstationDatabase& database,
                                                ModularEquipmentSystem& equipmentSystem, WorkstationType stationType)
    {
        WorkstationUIState state;
        state.stationType = stationType;
        state.visibleRecipes = equipmentSystem.getRecipesForStation(stationType);
        state.isOpen = true;
        state.availability = StationAvailability::Available;

        const WorkstationDef* def = database.findByType(stationType);

        if (def != nullptr)
        {
            state.objectID = def->objectID;
            state.title = !def->uiTitle.empty() ? def->uiTitle : FallbackTitleFor(stationType);
            state.supportedActions = def->supportedActions;
        }
        else
        {
            state.title = FallbackTitleFor(stationType);
            state.supportedActions.clear();
        }

        return state;
    }

    void SelectStationAction(WorkstationUIState& state, int actionIndex)
    {
        if (actionIndex < 0 || actionIndex >= static_cast<int>(state.supportedActions.size()))
        {
            state.selectedActionIndex = -1;
            state.selectedRecipeIndex = -1;
            state.statusMessage = "Invalid action selection.";
            return;
        }

        state.selectedActionIndex = actionIndex;
        state.selectedRecipeIndex = -1;
        state.statusMessage.clear();
    }

    void SelectRecipe(WorkstationUIState& state, int recipeIndex)
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

    void HandleCraftAction(WorkstationUIState& state, ModularEquipmentSystem& equipmentSystem,
                           PlayerInventory& inventory)
    {
        if (state.selectedRecipeIndex < 0 || state.selectedRecipeIndex >= static_cast<int>(state.visibleRecipes.size()))
        {
            state.statusMessage = "No recipe selected.";
            return;
        }

        const CraftingRecipe& localRecipe = state.visibleRecipes[state.selectedRecipeIndex];
        const auto& allRecipes = equipmentSystem.recipes();
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

        switch (equipmentSystem.craftItem(inventory, realIndex))
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
        default:
            break;
        }
    }

    void HandleRepairAction(WorkstationUIState& state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleModifyAction(WorkstationUIState& state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleScrapAction(WorkstationUIState& state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleDiagnoseAction(WorkstationUIState& state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleAccessTerminalAction(WorkstationUIState& state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void HandleOpenStorageAction(WorkstationUIState& state)
    {
        state.statusMessage = "Logic placeholder";
    }

    void ExecuteSelectedAction(WorkstationUIState& state, ModularEquipmentSystem& equipmentSystem,
                               PlayerInventory& inventory)
    {
        if (state.availability != StationAvailability::Available)
        {
            state.statusMessage = "Station is not available right now.";
            return;
        }

        if (state.selectedActionIndex < 0 ||
            state.selectedActionIndex >= static_cast<int>(state.supportedActions.size()))
        {
            state.statusMessage = "No action selected.";
            return;
        }

        switch (state.supportedActions[state.selectedActionIndex])
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
        default:
            break;
        }
    }

    WorkstationUIState OpenWorkstationUI(const WorkstationDatabase& database, ModularEquipmentSystem& equipmentSystem,
                                         WorkstationType stationType)
    {
        WorkstationUIState state = CreateWorkstationUIState(database, equipmentSystem, stationType);

        bunker::logInfo() << "=== " << state.title << " ===" << std::endl;

        if (state.supportedActions.empty())
        {
            bunker::logInfo() << "(no actions available)" << std::endl;
        }
        else
        {
            bunker::logInfo() << "Actions:" << std::endl;
            for (std::size_t i = 0; i < state.supportedActions.size(); ++i)
            {
                bunker::logInfo() << "  [" << i << "] " << ActionName(state.supportedActions[i]) << std::endl;
            }
        }

        if (state.selectedActionIndex >= 0 &&
            state.selectedActionIndex < static_cast<int>(state.supportedActions.size()) &&
            state.supportedActions[state.selectedActionIndex] == StationActionType::Craft)
        {
            bunker::logInfo() << "Recipes:" << std::endl;
            for (std::size_t i = 0; i < state.visibleRecipes.size(); ++i)
            {
                bunker::logInfo() << "  [" << i << "] " << state.visibleRecipes[i].resultName << std::endl;
            }
        }

        if (!state.statusMessage.empty())
        {
            bunker::logInfo() << "Status: " << state.statusMessage << std::endl;
        }

        return state;
    }

    WorkstationUIState OpenWorkstationUIFromWorld(const WorkstationDatabase& database,
                                                  const WorldWorkstation& worldStation,
                                                  ModularEquipmentSystem& equipmentSystem)
    {
        const WorkstationDef* def = database.findByID(worldStation.objectID);

        if (def == nullptr)
        {
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
        default:
            break;
        }

        return state;
    }
} // namespace bunker
