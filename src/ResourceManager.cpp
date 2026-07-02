#include "ResourceManager.hpp"

ResourceManager &ResourceManager::Get()
{
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::HasResources(int scrap, int circuits, int core) const
{
    return this->scrapMetal >= scrap &&
           this->circuits >= circuits &&
           this->coreEnergy >= core;
}

bool ResourceManager::ConsumeResources(int scrap, int circuits, int core)
{
    if (!HasResources(scrap, circuits, core))
        return false;

    scrapMetal -= scrap;
    circuits -= circuits;
    coreEnergy -= core;
    return true;
}

void ResourceManager::AddResources(int scrap, int circuits, int core)
{
    scrapMetal += scrap;
    circuits += circuits;
    coreEnergy += core;
}