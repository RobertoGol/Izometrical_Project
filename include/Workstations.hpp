#pragma once

#include "WorkstationTypes.hpp"
#include "WorkstationUIState.hpp"
#include "WorkstationDatabase.hpp"
#include <vector>

class ModularEquipmentSystem; // forward declaration

class Workstations
{
public:
    static WorkstationUIState CreateWorkstationUIState(WorkstationType stationType);
    static WorkstationUIState OpenWorkstationUI(WorkstationType stationType);

    // Action handlers
    static void HandleCraftAction(WorkstationUIState &state, int recipeIndex);
    static void HandleRepairAction(WorkstationUIState &state);
    static void HandleModifyAction(WorkstationUIState &state);
    static void HandleScrapAction(WorkstationUIState &state);
    static void HandleDiagnoseAction(WorkstationUIState &state);
    static void HandleAccessTerminalAction(WorkstationUIState &state);
    static void HandleOpenStorageAction(WorkstationUIState &state);

    static void DispatchStationAction(WorkstationUIState &state, StationActionType action, int recipeIndex = -1);

    // Recipe helpers (заглушка)
    static std::vector<CraftingRecipe> getRecipesForStation(WorkstationType type);
};