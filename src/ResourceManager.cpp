#include "ResourceManager.hpp"

ResourceManager &ResourceManager::Get()
{
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::HasResources(int scrap, int circuitCost, int core) const
{
    return this->scrapMetal >= scrap &&
           this->circuits >= circuitCost &&
           this->coreEnergy >= core;
}

bool ResourceManager::ConsumeResources(int scrap, int circuitCost, int core)
{
    if (!HasResources(scrap, circuitCost, core))
        return false;

    scrapMetal -= scrap;
    this->circuits -= circuitCost;
    coreEnergy -= core;
    return true;
}

void ResourceManager::AddResources(int scrap, int circuitAmount, int core)
{
    scrapMetal += scrap;
    circuits += circuitAmount;
    coreEnergy += core;
}
