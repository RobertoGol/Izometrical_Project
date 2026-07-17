#include "persistence/SaveSystem.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>

namespace bunker
{
    namespace
    {
        constexpr unsigned int MIN_SUPPORTED_SAVE_VERSION = 17;
        constexpr std::size_t MAX_SAVE_STRING_LENGTH = 4096;

        template <typename T>
        bool writeBlock(std::ofstream& file, const T& value)
        {
            file.write(reinterpret_cast<const char*>(&value), sizeof(T));
            return static_cast<bool>(file);
        }

        bool writeBytes(std::ofstream& file, const char* data, std::size_t size)
        {
            if (size == 0)
            {
                return true;
            }
            file.write(data, static_cast<std::streamsize>(size));
            return static_cast<bool>(file);
        }

        template <typename T>
        bool readBlock(std::ifstream& file, T& value, const std::string& label)
        {
            file.read(reinterpret_cast<char*>(&value), sizeof(T));
            if (file)
            {
                return true;
            }

            bunker::logError() << "[SAVE] Не удалось прочитать блок: " << label << std::endl;
            return false;
        }

        bool readBytes(std::ifstream& file, char* data, std::size_t size, const std::string& label)
        {
            if (size == 0)
            {
                return true;
            }

            file.read(data, static_cast<std::streamsize>(size));
            if (file)
            {
                return true;
            }

            bunker::logError() << "[SAVE] Не удалось прочитать данные: " << label << std::endl;
            return false;
        }

        RuntimeWorldSaveData makeRuntimeWorldSaveData(const Registry* registry)
        {
            RuntimeWorldSaveData data;
            if (registry != nullptr)
            {
                const auto& meshes = registry->meshes.getRawData();
                const auto& meshEntities = registry->meshes.getDenseEntities();
                data.ecsEntityCount = static_cast<std::uint32_t>(std::min(meshes.size(), meshEntities.size()));
            }
            return data;
        }
    }

    bool SaveSystem::writeSave(unsigned int slot, const GameState& gs, const PlayerInventory& inventory,
                               const Registry* registry)
    {
        std::filesystem::create_directories("saves");

        std::string path = "saves/slot_" + std::to_string(slot) + ".sav";
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            bunker::logError() << "[SAVE] Не удалось записать: " << path << std::endl;
            return false;
        }

        SaveFileHeader header;
        header.slotIndex = slot;
        if (!writeBlock(file, header))
            return false;

        PlayerSaveData psd;
        psd.position = gs.playerPos;
        psd.currentMode = gs.playerMode;
        psd.health = gs.playerHealth;
        psd.maxHealth = gs.playerMaxHealth;
        psd.erosionLevel = gs.playerErosionLevel;
        psd.currentScore = gs.score;
        if (!writeBlock(file, psd))
            return false;

        const auto& slots = inventory.getSlots();
        size_t invSize = slots.size();
        if (!writeBlock(file, invSize))
            return false;

        for (const auto& item : slots)
        {
            if (!writeBlock(file, item.itemID) || !writeBlock(file, item.type) || !writeBlock(file, item.quantity) ||
                !writeBlock(file, item.weightPerUnit))
                return false;

            size_t nameLen = item.displayName.size();
            if (!writeBlock(file, nameLen) || !writeBytes(file, item.displayName.data(), nameLen))
                return false;
        }

        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (!writeBlock(file, gs.sectorMap[x][y]) || !writeBlock(file, gs.wallDurability[x][y]) ||
                    !writeBlock(file, gs.etherErosionMap[x][y]))
                    return false;
            }
        }

        if (!writeBlock(file, gs.bunkerProgression) || !writeBlock(file, gs.story) ||
            !writeBlock(file, gs.characterProg.level) || !writeBlock(file, gs.characterProg.experience) ||
            !writeBlock(file, gs.characterProg.unusedPoints) || !writeBlock(file, gs.characterProg.sanityLine) ||
            !writeBlock(file, gs.characterProg.soulLine) || !writeBlock(file, gs.mapMeta.isBaseCleared) ||
            !writeBlock(file, gs.mapMeta.baseSuppliesLevel) || !writeBlock(file, gs.mapMeta.activeVerminNests) ||
            !writeBlock(file, gs.titan.position) || !writeBlock(file, gs.titan.health) ||
            !writeBlock(file, gs.titan.systems))
            return false;

        RuntimeWorldSaveData runtimeData = makeRuntimeWorldSaveData(registry);
        if (!writeBlock(file, runtimeData))
            return false;

        if (registry != nullptr)
        {
            const auto& meshes = registry->meshes.getRawData();
            const auto& meshEntities = registry->meshes.getDenseEntities();
            const std::size_t drawCount = std::min(meshes.size(), meshEntities.size());
            for (std::size_t i = 0; i < drawCount; ++i)
            {
                const TransformComponent* transform = registry->transforms.get(meshEntities[i]);
                if (transform == nullptr)
                {
                    RenderEntitySaveData empty;
                    empty.entity = meshEntities[i];
                    empty.materialID = meshes[i].materialID;
                    if (!writeBlock(file, empty))
                        return false;
                    continue;
                }

                RenderEntitySaveData entityData;
                entityData.entity = meshEntities[i];
                entityData.transform = *transform;
                entityData.materialID = meshes[i].materialID;
                if (!writeBlock(file, entityData))
                    return false;
            }
        }

        file.close();
        bunker::logInfo() << "[SAVE] Сохранено в " << path << std::endl;
        return true;
    }

    bool SaveSystem::readSave(unsigned int slot, GameState& gs, PlayerInventory& inventory, Registry* registry)
    {
        std::string path = "saves/slot_" + std::to_string(slot) + ".sav";
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            bunker::logInfo() << "[SAVE] Файл не найден: " << path << std::endl;
            return false;
        }

        SaveFileHeader header;
        if (!readBlock(file, header, "header"))
            return false;

        if (header.magic[0] != 'B' || header.magic[1] != 'S' || header.magic[2] != 'A' || header.magic[3] != 'V')
        {
            bunker::logError() << "[SAVE] Повреждён файл: " << path << std::endl;
            file.close();
            return false;
        }

        if (header.version < MIN_SUPPORTED_SAVE_VERSION || header.version > SAVE_FORMAT_VERSION)
        {
            bunker::logError() << "[SAVE] Несовместимая версия формата сохранения: " << header.version
                               << " (поддерживается " << MIN_SUPPORTED_SAVE_VERSION << "-" << SAVE_FORMAT_VERSION
                               << ")" << std::endl;
            file.close();
            return false;
        }

        PlayerSaveData psd;
        if (!readBlock(file, psd, "player"))
            return false;

        gs.playerPos = psd.position;
        gs.playerMode = psd.currentMode;
        gs.playerHealth = psd.health;
        gs.playerMaxHealth = psd.maxHealth;
        gs.playerErosionLevel = psd.erosionLevel;
        gs.score = psd.currentScore;

        inventory.clear();
        size_t invSize = 0;
        if (!readBlock(file, invSize, "inventory size"))
            return false;

        for (size_t i = 0; i < invSize; ++i)
        {
            unsigned int id = 0;
            ItemType type = ItemType::Things;
            int qty = 0;
            float weight = 0.0f;
            if (!readBlock(file, id, "inventory item id") || !readBlock(file, type, "inventory item type") ||
                !readBlock(file, qty, "inventory item quantity") || !readBlock(file, weight, "inventory item weight"))
                return false;

            size_t nameLen = 0;
            if (!readBlock(file, nameLen, "inventory item name length"))
                return false;
            if (nameLen > MAX_SAVE_STRING_LENGTH)
            {
                bunker::logError() << "[SAVE] Слишком длинная строка предмета: " << nameLen << std::endl;
                return false;
            }
            std::string name(nameLen, '\0');
            if (!readBytes(file, name.data(), nameLen, "inventory item name"))
                return false;

            inventory.addItem(id, type, qty, weight, name);
        }

        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (!readBlock(file, gs.sectorMap[x][y], "sector map") ||
                    !readBlock(file, gs.wallDurability[x][y], "wall durability") ||
                    !readBlock(file, gs.etherErosionMap[x][y], "erosion map"))
                    return false;
            }
        }

        if (!readBlock(file, gs.bunkerProgression, "bunker progression") || !readBlock(file, gs.story, "story") ||
            !readBlock(file, gs.characterProg.level, "character level") ||
            !readBlock(file, gs.characterProg.experience, "character experience") ||
            !readBlock(file, gs.characterProg.unusedPoints, "character points") ||
            !readBlock(file, gs.characterProg.sanityLine, "sanity line") ||
            !readBlock(file, gs.characterProg.soulLine, "soul line") ||
            !readBlock(file, gs.mapMeta.isBaseCleared, "base cleared") ||
            !readBlock(file, gs.mapMeta.baseSuppliesLevel, "base supplies") ||
            !readBlock(file, gs.mapMeta.activeVerminNests, "vermin nests") ||
            !readBlock(file, gs.titan.position, "titan position") || !readBlock(file, gs.titan.health, "titan health") ||
            !readBlock(file, gs.titan.systems, "titan systems"))
            return false;

        if (header.version >= 18)
        {
            RuntimeWorldSaveData runtimeData;
            if (!readBlock(file, runtimeData, "runtime world metadata"))
                return false;

            for (std::uint32_t i = 0; i < runtimeData.ecsEntityCount; ++i)
            {
                RenderEntitySaveData entityData;
                if (!readBlock(file, entityData, "render entity"))
                    return false;

                if (registry == nullptr)
                {
                    continue;
                }

                if (auto* transform = registry->transforms.get(entityData.entity))
                {
                    *transform = entityData.transform;
                }
                if (auto* mesh = registry->meshes.get(entityData.entity))
                {
                    mesh->materialID = entityData.materialID;
                }
            }
        }

        file.close();
        bunker::logInfo() << "[SAVE] Загружено из " << path << std::endl;
        return true;
    }

    bool SaveSystem::saveExists(unsigned int slot)
    {
        std::string path = "saves/slot_" + std::to_string(slot) + ".sav";
        return std::filesystem::exists(path);
    }

} // namespace bunker
