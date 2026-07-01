#pragma once

#include "content/Workstations.hpp"
#include "persistence/ModularEquipmentSystem.hpp"

namespace bunker
{
    void OpenWorkstationUI(ModularEquipmentSystem &equipmentSystem,
                           WorkstationType stationType);
}