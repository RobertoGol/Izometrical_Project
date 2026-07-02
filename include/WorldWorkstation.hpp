#pragma once

#include "WorkstationTypes.hpp"
#include <string>

struct WorldWorkstation
{
    int objectID = -1;
    WorkstationType type = WorkstationType::None;

    float x = 0.0f;
    float y = 0.0f;

    int currentHealth = 100;
    int maxHealth = 100;

    bool isPowered = true;
    bool isDestroyed = false;
    bool isActive = true;

    std::string spriteTag;

    bool CanUse() const
    {
        return !isDestroyed && isActive && isPowered;
    }

    StationAvailability GetAvailability() const;
};