#include "WorldWorkstation.hpp"

StationAvailability WorldWorkstation::GetAvailability() const
{
    if (isDestroyed)
        return StationAvailability::Destroyed;
    if (!isActive)
        return StationAvailability::Inactive;
    if (!isPowered)
        return StationAvailability::NoPower;
    return StationAvailability::Available;
}