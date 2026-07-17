#pragma once

#include <string>
#include <vector>

enum class WorkstationType
{
    None = 0,
    ArmorWorkbench,
    WeaponsWorkbench,
    TinkerWorkbench,
    ChemStation,
    TankMaintenanceBay,
    Terminal,
    Storage
};

enum class StationActionType
{
    None = 0,
    Craft,
    Repair,
    Modify,
    Scrap,
    Diagnose,
    AccessTerminal,
    OpenStorage
};

enum class StationAvailability
{
    Available,
    Inactive,
    Destroyed,
    NoPower
};

struct WorkstationDef
{
    WorkstationType type = WorkstationType::None;
    std::string uiTitle;
    std::vector<StationActionType> supportedActions;
    bool requiresPower = true;
    int powerConsumption = 0;
};

struct CraftingRecipe
{
    int id = -1;
    std::string name;
    int requiredScrap = 0;
    int requiredCircuits = 0;
    int requiredCoreEnergy = 0;
    WorkstationType stationType = WorkstationType::None;
};