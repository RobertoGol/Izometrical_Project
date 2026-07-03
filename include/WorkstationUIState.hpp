#pragma once

#include "WorkstationTypes.hpp"
#include <string>
#include <vector>

struct WorkstationUIState
{
    std::string uiTitle = "Workstation";
    std::vector<StationActionType> supportedActions;
    int selectedActionIndex = -1;
    int selectedRecipeIndex = -1;
    std::string statusMessage;
    bool isAvailable = true;
};