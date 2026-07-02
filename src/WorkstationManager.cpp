#include "WorkstationManager.hpp"
#include <algorithm>
#include <cmath>

WorkstationManager &WorkstationManager::Get()
{
    static WorkstationManager instance;
    return instance;
}

WorldWorkstation *WorkstationManager::SpawnWorkstation(WorkstationType type, float x, float y)
{
    auto ws = std::make_unique<WorldWorkstation>();
    ws->objectID = nextObjectID++;
    ws->type = type;
    ws->x = x;
    ws->y = y;
    ws->spriteTag = "station_" + std::to_string(static_cast<int>(type));

    WorldWorkstation *ptr = ws.get();
    stations.push_back(std::move(ws));
    return ptr;
}

WorldWorkstation *WorkstationManager::FindByObjectID(int objectID)
{
    for (auto &ws : stations)
    {
        if (ws->objectID == objectID)
            return ws.get();
    }
    return nullptr;
}

WorldWorkstation *WorkstationManager::FindNearest(float x, float y, float maxDistance)
{
    WorldWorkstation *closest = nullptr;
    float minDist = maxDistance;

    for (auto &ws : stations)
    {
        float dx = ws->x - x;
        float dy = ws->y - y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist)
        {
            minDist = dist;
            closest = ws.get();
        }
    }
    return closest;
}

void WorkstationManager::DestroyWorkstation(int objectID)
{
    auto it = std::remove_if(stations.begin(), stations.end(),
                             [objectID](const auto &ws)
                             { return ws->objectID == objectID; });
    stations.erase(it, stations.end());
}

void WorkstationManager::SaveAll()
{
    // TODO: Сериализация в файл
}

void WorkstationManager::LoadAll()
{
    // TODO: Десериализация
}

const std::vector<std::unique_ptr<WorldWorkstation>> &WorkstationManager::GetAllStations() const
{
    return stations;
}