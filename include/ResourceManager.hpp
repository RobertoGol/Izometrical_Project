#pragma once

class ResourceManager
{
public:
    static ResourceManager &Get();

    // Resources
    int scrapMetal = 100;
    int circuits = 20;
    int coreEnergy = 15;

    bool HasResources(int scrap, int circuitCost, int core) const;
    bool ConsumeResources(int scrap, int circuitCost, int core);
    void AddResources(int scrap, int circuitAmount, int core);

private:
    ResourceManager() = default;
};
