#pragma once

#include "WorldWorkstation.hpp"
#include "WorkstationUIState.hpp"
#include <vector>
#include <memory>

class WorkstationManager
{
public:
    static WorkstationManager &Get();

    WorldWorkstation *SpawnWorkstation(WorkstationType type, float x, float y);
    WorldWorkstation *FindByObjectID(int objectID);
    WorldWorkstation *FindNearest(float x, float y, float maxDistance = 5.0f);

    void DestroyWorkstation(int objectID);
    void Clear();
    void SaveAll();
    void LoadAll();

    const std::vector<std::unique_ptr<WorldWorkstation>> &GetAllStations() const;

    // === Новые методы для UI ===
    WorkstationUIState OpenUI(int objectID);
    WorkstationUIState OpenUI(WorldWorkstation *station);

private:
    WorkstationManager() = default;
    std::vector<std::unique_ptr<WorldWorkstation>> stations;
    int nextObjectID = 1;
};
