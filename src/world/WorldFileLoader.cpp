#include "world/WorldFileLoader.hpp"

#include "WorkstationManager.hpp"
#include "engine/Log.hpp"
#include "world/DoorTransition.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace bunker
{
    namespace
    {
        std::string displayName(std::string value)
        {
            std::replace(value.begin(), value.end(), '_', ' ');
            return value;
        }

        ItemType parseItemType(const std::string& value)
        {
            if (value == "Weapon")
                return ItemType::Weapon;
            if (value == "Armor")
                return ItemType::Armor;
            if (value == "Medicine")
                return ItemType::Medicine;
            if (value == "Ammo")
                return ItemType::Ammo;
            if (value == "Things")
                return ItemType::Things;
            if (value == "Quest")
                return ItemType::Quest;
            return ItemType::Resource;
        }

        LootContainerType parseContainerType(const std::string& value)
        {
            if (value == "IronSafe")
                return LootContainerType::IronSafe;
            if (value == "DevVault")
                return LootContainerType::DevVault;
            return LootContainerType::WoodenCrate;
        }

        WorkstationType parseWorkstationType(const std::string& value)
        {
            if (value == "ArmorWorkbench")
                return WorkstationType::ArmorWorkbench;
            if (value == "WeaponsWorkbench")
                return WorkstationType::WeaponsWorkbench;
            if (value == "TinkerWorkbench")
                return WorkstationType::TinkerWorkbench;
            if (value == "ChemStation")
                return WorkstationType::ChemStation;
            if (value == "TankMaintenanceBay")
                return WorkstationType::TankMaintenanceBay;
            if (value == "Terminal")
                return WorkstationType::Terminal;
            if (value == "Storage")
                return WorkstationType::Storage;
            return WorkstationType::None;
        }

        void resetWorldState(GameState& gameState)
        {
            gameState.lootContainers.clear();
            gameState.loosePickups.clear();
            gameState.neutralNpcs.clear();
            gameState.enemies.clear();
            gameState.bullets.clear();
            WorkstationManager::Get().Clear();

            for (int x = 0; x < Config::MAP_WIDTH; ++x)
            {
                for (int y = 0; y < Config::MAP_HEIGHT; ++y)
                {
                    gameState.sectorMap[x][y] = 0;
                    gameState.wallDurability[x][y] = 0;
                    gameState.etherErosionMap[x][y] = 0.0f;
                }
            }
        }

        void applyRow(GameState& gameState, int y, const std::string& row)
        {
            if (y < 0 || y >= Config::MAP_HEIGHT)
            {
                return;
            }

            const int width = std::min(static_cast<int>(row.size()), Config::MAP_WIDTH);
            for (int x = 0; x < width; ++x)
            {
                switch (row[static_cast<std::size_t>(x)])
                {
                case '#':
                    gameState.sectorMap[x][y] = 1;
                    gameState.wallDurability[x][y] = 100;
                    break;
                case '~':
                    gameState.sectorMap[x][y] = 3;
                    gameState.etherErosionMap[x][y] = 25.0f;
                    break;
                default:
                    gameState.sectorMap[x][y] = 0;
                    gameState.wallDurability[x][y] = 0;
                    break;
                }
            }
        }
    } // namespace

    std::string WorldFileLoader::pathForWorldId(const std::string& worldId)
    {
        return (std::filesystem::path("assets/worlds") / (worldId + ".bwld")).string();
    }

    bool WorldFileLoader::load(const std::string& path, GameState& gameState, DoorTransition* doorTransition)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            bunker::logInfo() << "[WORLD] .bwld not found: " << path << std::endl;
            return false;
        }

        resetWorldState(gameState);

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            std::istringstream row(line);
            std::string tag;
            row >> tag;

            if (tag == "map")
            {
                row >> gameState.mapMeta.currentMapName;
            }
            else if (tag == "row")
            {
                int y = 0;
                std::string cells;
                if (row >> y >> cells)
                {
                    applyRow(gameState, y, cells);
                }
            }
            else if (tag == "player")
            {
                row >> gameState.playerPos.x >> gameState.playerPos.y >> gameState.playerPos.z;
                gameState.cameraTarget = gameState.playerPos;
            }
            else if (tag == "titan")
            {
                row >> gameState.titan.position.x >> gameState.titan.position.y >> gameState.titan.position.z;
            }
            else if (tag == "tower")
            {
                row >> gameState.towerPosition.x >> gameState.towerPosition.y >> gameState.towerPosition.z;
            }
            else if (tag == "erosion")
            {
                int x = 0;
                int y = 0;
                float value = 0.0f;
                if (row >> x >> y >> value && x >= 0 && x < Config::MAP_WIDTH && y >= 0 && y < Config::MAP_HEIGHT)
                {
                    gameState.etherErosionMap[x][y] = value;
                }
            }
            else if (tag == "container")
            {
                LootContainer container;
                std::string type;
                if (row >> container.position.x >> container.position.y >> type >> container.respawnDelaySeconds)
                {
                    container.position.z = 0.0f;
                    container.type = parseContainerType(type);
                    gameState.lootContainers.push_back(container);
                }
            }
            else if (tag == "pickup")
            {
                LoosePickup pickup;
                std::string itemType;
                std::string name;
                if (row >> pickup.position.x >> pickup.position.y >> pickup.item.itemID >> itemType >>
                    pickup.item.quantity >> pickup.item.weightPerUnit >> name)
                {
                    pickup.position.z = 0.0f;
                    pickup.item.type = parseItemType(itemType);
                    pickup.item.displayName = displayName(name);
                    gameState.loosePickups.push_back(pickup);
                }
            }
            else if (tag == "npc")
            {
                NeutralNpc npc;
                std::string name;
                if (row >> npc.id >> npc.position.x >> npc.position.y >> name)
                {
                    npc.position.z = 0.0f;
                    npc.displayName = displayName(name);
                    gameState.neutralNpcs.push_back(npc);
                }
            }
            else if (tag == "door")
            {
                DoorLink door;
                if (row >> door.id >> door.triggerPosition.x >> door.triggerPosition.y >> door.linkTarget >>
                    door.destinationSpawnPoint.x >> door.destinationSpawnPoint.y >> door.activationRadius)
                {
                    door.triggerPosition.z = 0.0f;
                    door.destinationSpawnPoint.z = 0.0f;
                    if (doorTransition != nullptr)
                    {
                        doorTransition->registerDoor(door);
                    }
                }
            }
            else if (tag == "workstation")
            {
                std::string type;
                float x = 0.0f;
                float y = 0.0f;
                if (row >> type >> x >> y)
                {
                    const WorkstationType stationType = parseWorkstationType(type);
                    if (stationType != WorkstationType::None)
                    {
                        WorkstationManager::Get().SpawnWorkstation(stationType, x, y);
                    }
                }
            }
        }

        bunker::logInfo() << "[WORLD] Loaded .bwld: " << path << std::endl;
        return true;
    }
} // namespace bunker
