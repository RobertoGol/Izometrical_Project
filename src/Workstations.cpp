#include "Workstations.hpp"
#include "RecipeDatabase.hpp"
#include "ResourceManager.hpp"
#include <iostream>

WorkstationUIState Workstations::CreateWorkstationUIState(WorkstationType stationType)
{
    WorkstationUIState state;
    const WorkstationDef *def = WorkstationDatabase::findByType(stationType);

    if (def)
    {
        state.uiTitle = def->uiTitle;
        state.supportedActions = def->supportedActions;
        state.isAvailable = true;
    }
    else
    {
        state.uiTitle = "Unknown Workstation";
        state.isAvailable = false;
        state.statusMessage = "Station definition not found.";
    }
    return state;
}

WorkstationUIState Workstations::OpenWorkstationUI(WorkstationType stationType)
{
    WorkstationUIState state = CreateWorkstationUIState(stationType);

    if (!state.isAvailable)
    {
        return state;
    }

    // Показываем список доступных действий
    std::cout << "=== " << state.uiTitle << " ===" << std::endl;
    for (size_t i = 0; i < state.supportedActions.size(); ++i)
    {
        std::cout << i << ". Action: " << static_cast<int>(state.supportedActions[i]) << std::endl;
    }

    state.statusMessage = "Select an action.";
    return state;
}

void Workstations::DispatchStationAction(WorkstationUIState &state, StationActionType action, int recipeIndex)
{
    switch (action)
    {
    case StationActionType::Craft:
        HandleCraftAction(state, recipeIndex);
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
        state.statusMessage = "Unknown action.";
        break;
    }
}

void Workstations::HandleCraftAction(WorkstationUIState &state, int recipeIndex)
{
    if (recipeIndex < 0)
    {
        state.statusMessage = "Select a recipe to craft.";
        return;
    }

    // Используем реальную базу рецептов
    auto recipes = RecipeDatabase::Get().GetRecipesForStation(WorkstationType::ArmorWorkbench);
    if (recipeIndex >= recipes.size())
    {
        state.statusMessage = "Invalid recipe index.";
        return;
    }

    const auto &recipe = recipes[recipeIndex];

    auto &resources = ResourceManager::Get();
    if (!resources.HasResources(recipe.requiredScrap, recipe.requiredCircuits, recipe.requiredCoreEnergy))
    {
        state.statusMessage = "Not enough resources.";
        return;
    }

    if (resources.ConsumeResources(recipe.requiredScrap, recipe.requiredCircuits, recipe.requiredCoreEnergy))
    {
        state.statusMessage = "Crafted: " + recipe.name;
        state.selectedRecipeIndex = recipeIndex;
    }
    else
    {
        state.statusMessage = "Failed to consume resources.";
    }
}

void Workstations::HandleRepairAction(WorkstationUIState &state)
{
    state.statusMessage = "Repair action executed.";
}

void Workstations::HandleModifyAction(WorkstationUIState &state)
{
    state.statusMessage = "Modify action executed.";
}

void Workstations::HandleScrapAction(WorkstationUIState &state)
{
    state.statusMessage = "Scrap action executed.";
}

void Workstations::HandleDiagnoseAction(WorkstationUIState &state)
{
    state.statusMessage = "Diagnosis complete.";
}

void Workstations::HandleAccessTerminalAction(WorkstationUIState &state)
{
    state.statusMessage = "Terminal access granted.";
}

void Workstations::HandleOpenStorageAction(WorkstationUIState &state)
{
    state.statusMessage = "Storage opened.";
}

std::vector<CraftingRecipe> Workstations::getRecipesForStation(WorkstationType type)
{
    return RecipeDatabase::Get().GetRecipesForStation(type);
}