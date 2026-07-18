#include "persistence/SaveGame.hpp"

#include "persistence/ModularEquipmentSystem.hpp"
#include "persistence/SaveSystem.hpp"
#include "world/DoorTransition.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace bunker
{
    namespace
    {
        std::string runtimePath(unsigned int slot)
        {
            return "saves/slot_" + std::to_string(slot) + ".runtime";
        }

        void writePart(std::ofstream& file, const char* key, const TankPartPlating& part)
        {
            file << key << '=' << part.currentHp << '\n';
        }

        void writeRuntimeWorldState(std::ofstream& file, const GameState& gameState)
        {
            file << "containers=";
            for (std::size_t i = 0; i < gameState.lootContainers.size(); ++i)
            {
                const LootContainer& container = gameState.lootContainers[i];
                if (i > 0)
                {
                    file << ';';
                }
                file << i << ',' << (container.isOpened ? 1 : 0) << ',' << container.respawnTimerSeconds << ','
                     << container.respawnDelaySeconds;
            }
            file << '\n';

            file << "collectedPickups=";
            bool wrotePickup = false;
            for (std::size_t i = 0; i < gameState.loosePickups.size(); ++i)
            {
                if (!gameState.loosePickups[i].collected)
                {
                    continue;
                }
                file << (wrotePickup ? "," : "") << i;
                wrotePickup = true;
            }
            file << '\n';

            file << "talkedNpcs=";
            bool wroteNpc = false;
            for (std::size_t i = 0; i < gameState.neutralNpcs.size(); ++i)
            {
                if (!gameState.neutralNpcs[i].hasTalked)
                {
                    continue;
                }
                file << (wroteNpc ? "," : "") << i;
                wroteNpc = true;
            }
            file << '\n';
        }

        bool writeRuntimeSidecar(const SaveGameContext& context)
        {
            if (context.gameState == nullptr && context.doorTransition == nullptr && context.modularEquipment == nullptr)
            {
                return true;
            }

            std::filesystem::create_directories("saves");
            std::ofstream file(runtimePath(context.slot), std::ios::trunc);
            if (!file.is_open())
            {
                return false;
            }

            if (context.doorTransition != nullptr)
            {
                file << "world=" << context.doorTransition->activeWorldId() << '\n';
                file << "openedDoors=";
                const std::vector<int>& opened = context.doorTransition->openedDoorIds();
                for (std::size_t i = 0; i < opened.size(); ++i)
                {
                    if (i > 0)
                    {
                        file << ',';
                    }
                    file << opened[i];
                }
                file << '\n';
            }

            if (context.gameState != nullptr)
            {
                writeRuntimeWorldState(file, *context.gameState);
            }

            if (context.modularEquipment != nullptr)
            {
                const ModularTankChassis& tank = context.modularEquipment->titanRuntimeChassis();
                writePart(file, "leftTrackHp", tank.leftTrack);
                writePart(file, "rightTrackHp", tank.rightTrack);
                writePart(file, "coreReactorHp", tank.coreReactor);
                writePart(file, "mainCannonHp", tank.mainCannon);
                writePart(file, "frontalArmorHp", tank.frontalArmor);
                writePart(file, "rearArmorHp", tank.rearArmor);
            }

            return static_cast<bool>(file);
        }

        std::vector<std::string> split(const std::string& value, char delimiter)
        {
            std::vector<std::string> tokens;
            std::stringstream stream(value);
            std::string token;
            while (std::getline(stream, token, delimiter))
            {
                if (!token.empty())
                {
                    tokens.push_back(token);
                }
            }
            return tokens;
        }

        std::vector<int> parseOpenedDoors(const std::string& value)
        {
            std::vector<int> ids;
            for (const std::string& token : split(value, ','))
            {
                try
                {
                    ids.push_back(std::stoi(token));
                }
                catch (...)
                {
                }
            }
            return ids;
        }

        bool tryParseFloat(const std::string& value, float& parsed)
        {
            try
            {
                parsed = std::stof(value);
            }
            catch (...)
            {
                return false;
            }

            return true;
        }

        bool tryParseSize(const std::string& value, std::size_t& parsed)
        {
            try
            {
                parsed = static_cast<std::size_t>(std::stoull(value));
            }
            catch (...)
            {
                return false;
            }

            return true;
        }

        bool tryParseInt(const std::string& value, int& parsed)
        {
            try
            {
                parsed = std::stoi(value);
            }
            catch (...)
            {
                return false;
            }

            return true;
        }

        void applyPartHp(const std::string& key, float value, ModularTankChassis& tank)
        {
            if (key == "leftTrackHp")
                tank.leftTrack.currentHp = value;
            else if (key == "rightTrackHp")
                tank.rightTrack.currentHp = value;
            else if (key == "coreReactorHp")
                tank.coreReactor.currentHp = value;
            else if (key == "mainCannonHp")
                tank.mainCannon.currentHp = value;
            else if (key == "frontalArmorHp")
                tank.frontalArmor.currentHp = value;
            else if (key == "rearArmorHp")
                tank.rearArmor.currentHp = value;
        }

        void applyContainerRuntime(const std::string& value, GameState& gameState)
        {
            for (const std::string& entry : split(value, ';'))
            {
                const std::vector<std::string> fields = split(entry, ',');
                if (fields.size() < 4)
                {
                    continue;
                }

                std::size_t index = 0;
                int opened = 0;
                float timer = 0.0f;
                float delay = 0.0f;
                if (!tryParseSize(fields[0], index) || !tryParseInt(fields[1], opened) ||
                    !tryParseFloat(fields[2], timer) || !tryParseFloat(fields[3], delay) ||
                    index >= gameState.lootContainers.size())
                {
                    continue;
                }

                LootContainer& container = gameState.lootContainers[index];
                container.isOpened = opened != 0;
                container.respawnTimerSeconds = timer;
                container.respawnDelaySeconds = delay;
            }
        }

        void applyPickupRuntime(const std::string& value, GameState& gameState)
        {
            for (auto& pickup : gameState.loosePickups)
            {
                pickup.collected = false;
            }

            for (const std::string& token : split(value, ','))
            {
                std::size_t index = 0;
                if (tryParseSize(token, index) && index < gameState.loosePickups.size())
                {
                    gameState.loosePickups[index].collected = true;
                }
            }
        }

        void applyNpcRuntime(const std::string& value, GameState& gameState)
        {
            for (auto& npc : gameState.neutralNpcs)
            {
                npc.hasTalked = false;
            }

            for (const std::string& token : split(value, ','))
            {
                std::size_t index = 0;
                if (tryParseSize(token, index) && index < gameState.neutralNpcs.size())
                {
                    gameState.neutralNpcs[index].hasTalked = true;
                }
            }
        }

        bool readRuntimeSidecar(const SaveGameContext& context)
        {
            std::ifstream file(runtimePath(context.slot));
            if (!file.is_open())
            {
                return true;
            }

            std::string activeWorldId;
            std::vector<int> openedDoors;
            std::string line;
            while (std::getline(file, line))
            {
                const std::size_t sep = line.find('=');
                if (sep == std::string::npos)
                {
                    continue;
                }

                const std::string key = line.substr(0, sep);
                const std::string value = line.substr(sep + 1);
                if (key == "world")
                {
                    activeWorldId = value;
                }
                else if (key == "openedDoors")
                {
                    openedDoors = parseOpenedDoors(value);
                }
                else if (key == "containers" && context.gameState != nullptr)
                {
                    applyContainerRuntime(value, *context.gameState);
                }
                else if (key == "collectedPickups" && context.gameState != nullptr)
                {
                    applyPickupRuntime(value, *context.gameState);
                }
                else if (key == "talkedNpcs" && context.gameState != nullptr)
                {
                    applyNpcRuntime(value, *context.gameState);
                }
                else if (context.modularEquipment != nullptr)
                {
                    float partHp = 0.0f;
                    if (tryParseFloat(value, partHp))
                    {
                        applyPartHp(key, partHp, context.modularEquipment->titanRuntimeChassis());
                    }
                }
            }

            if (context.doorTransition != nullptr)
            {
                context.doorTransition->restorePersistence(activeWorldId, openedDoors);
                if (context.gameState != nullptr && !activeWorldId.empty())
                {
                    context.gameState->mapMeta.currentMapName = activeWorldId;
                }
            }

            return true;
        }
    } // namespace

    bool SaveGame::exists(unsigned int slot)
    {
        return SaveSystem::saveExists(slot);
    }

    bool SaveGame::save(const SaveGameContext& context)
    {
        if (context.gameState == nullptr || context.inventory == nullptr)
        {
            return false;
        }

        return SaveSystem::writeSave(
                   context.slot, *context.gameState, *context.inventory, context.registry, context.advanced) &&
               writeRuntimeSidecar(context);
    }

    bool SaveGame::load(const SaveGameContext& context)
    {
        if (context.gameState == nullptr || context.inventory == nullptr)
        {
            return false;
        }

        return SaveSystem::readSave(
                   context.slot, *context.gameState, *context.inventory, context.registry, context.advanced) &&
               readRuntimeSidecar(context);
    }
} // namespace bunker
