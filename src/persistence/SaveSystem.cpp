#include "persistence/SaveSystem.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace bunker
{

    bool SaveSystem::writeSave(unsigned int slot, const GameState &gs, const PlayerInventory &inventory)
    {
        std::filesystem::create_directories("saves");

        std::string path = "saves/slot_" + std::to_string(slot) + ".sav";
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            std::cerr << "[SAVE] Не удалось записать: " << path << std::endl;
            return false;
        }

        SaveFileHeader header;
        header.slotIndex = slot;
        file.write(reinterpret_cast<const char *>(&header), sizeof(SaveFileHeader));

        PlayerSaveData psd;
        psd.position = gs.playerPos;
        psd.currentMode = gs.playerMode;
        psd.health = gs.playerHealth;
        psd.maxHealth = gs.playerMaxHealth;
        psd.erosionLevel = gs.playerErosionLevel;
        psd.currentScore = gs.score;
        file.write(reinterpret_cast<const char *>(&psd), sizeof(PlayerSaveData));

        const auto &slots = inventory.getSlots();
        size_t invSize = slots.size();
        file.write(reinterpret_cast<const char *>(&invSize), sizeof(size_t));

        for (const auto &item : slots)
        {
            file.write(reinterpret_cast<const char *>(&item.itemID), sizeof(unsigned int));
            file.write(reinterpret_cast<const char *>(&item.type), sizeof(ItemType));
            file.write(reinterpret_cast<const char *>(&item.quantity), sizeof(int));
            file.write(reinterpret_cast<const char *>(&item.weightPerUnit), sizeof(float));

            size_t nameLen = item.displayName.size();
            file.write(reinterpret_cast<const char *>(&nameLen), sizeof(size_t));
            file.write(item.displayName.data(), nameLen);
        }

        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                file.write(reinterpret_cast<const char *>(&gs.sectorMap[x][y]), sizeof(int));
                file.write(reinterpret_cast<const char *>(&gs.wallDurability[x][y]), sizeof(int));
                file.write(reinterpret_cast<const char *>(&gs.etherErosionMap[x][y]), sizeof(float));
            }
        }

        file.write(reinterpret_cast<const char *>(&gs.bunkerProgression), sizeof(Vault17Progression));
        file.write(reinterpret_cast<const char *>(&gs.story), sizeof(StoryState));
        file.write(reinterpret_cast<const char *>(&gs.characterProg.level), sizeof(int));
        file.write(reinterpret_cast<const char *>(&gs.characterProg.experience), sizeof(int));
        file.write(reinterpret_cast<const char *>(&gs.characterProg.unusedPoints), sizeof(int));
        file.write(reinterpret_cast<const char *>(&gs.characterProg.sanityLine), sizeof(float));
        file.write(reinterpret_cast<const char *>(&gs.characterProg.soulLine), sizeof(float));

        file.write(reinterpret_cast<const char *>(&gs.mapMeta.isBaseCleared), sizeof(bool));
        file.write(reinterpret_cast<const char *>(&gs.mapMeta.baseSuppliesLevel), sizeof(float));
        file.write(reinterpret_cast<const char *>(&gs.mapMeta.activeVerminNests), sizeof(unsigned int));

        file.write(reinterpret_cast<const char *>(&gs.titan.position), sizeof(Vector3D));
        file.write(reinterpret_cast<const char *>(&gs.titan.health), sizeof(float));
        file.write(reinterpret_cast<const char *>(&gs.titan.systems), sizeof(TitanComponents));

        file.close();
        std::cout << "[SAVE] Сохранено в " << path << std::endl;
        return true;
    }

    bool SaveSystem::readSave(unsigned int slot, GameState &gs, PlayerInventory &inventory)
    {
        std::string path = "saves/slot_" + std::to_string(slot) + ".sav";
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cout << "[SAVE] Файл не найден: " << path << std::endl;
            return false;
        }

        SaveFileHeader header;
        file.read(reinterpret_cast<char *>(&header), sizeof(SaveFileHeader));
        if (header.magic[0] != 'B' || header.magic[1] != 'S' ||
            header.magic[2] != 'A' || header.magic[3] != 'V')
        {
            std::cerr << "[SAVE] Повреждён файл: " << path << std::endl;
            file.close();
            return false;
        }

        if (header.version != 16)
        {
            std::cerr << "[SAVE] Несовместимая версия формата сохранения: " << header.version << " (ожидалась 16)" << std::endl;
            file.close();
            return false;
        }

        PlayerSaveData psd;
        file.read(reinterpret_cast<char *>(&psd), sizeof(PlayerSaveData));
        gs.playerPos = psd.position;
        gs.playerMode = psd.currentMode;
        gs.playerHealth = psd.health;
        gs.playerMaxHealth = psd.maxHealth;
        gs.playerErosionLevel = psd.erosionLevel;
        gs.score = psd.currentScore;

        inventory.clear();
        size_t invSize = 0;
        file.read(reinterpret_cast<char *>(&invSize), sizeof(size_t));

        for (size_t i = 0; i < invSize; ++i)
        {
            unsigned int id;
            ItemType type;
            int qty;
            float weight;
            file.read(reinterpret_cast<char *>(&id), sizeof(unsigned int));
            file.read(reinterpret_cast<char *>(&type), sizeof(ItemType));
            file.read(reinterpret_cast<char *>(&qty), sizeof(int));
            file.read(reinterpret_cast<char *>(&weight), sizeof(float));

            size_t nameLen = 0;
            file.read(reinterpret_cast<char *>(&nameLen), sizeof(size_t));
            std::string name(nameLen, '\0');
            file.read(&name[0], nameLen);

            inventory.addItem(id, type, qty, weight, name);
        }

        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                file.read(reinterpret_cast<char *>(&gs.sectorMap[x][y]), sizeof(int));
                file.read(reinterpret_cast<char *>(&gs.wallDurability[x][y]), sizeof(int));
                file.read(reinterpret_cast<char *>(&gs.etherErosionMap[x][y]), sizeof(float));
            }
        }

        file.read(reinterpret_cast<char *>(&gs.bunkerProgression), sizeof(Vault17Progression));
        file.read(reinterpret_cast<char *>(&gs.story), sizeof(StoryState));
        file.read(reinterpret_cast<char *>(&gs.characterProg.level), sizeof(int));
        file.read(reinterpret_cast<char *>(&gs.characterProg.experience), sizeof(int));
        file.read(reinterpret_cast<char *>(&gs.characterProg.unusedPoints), sizeof(int));
        file.read(reinterpret_cast<char *>(&gs.characterProg.sanityLine), sizeof(float));
        file.read(reinterpret_cast<char *>(&gs.characterProg.soulLine), sizeof(float));

        file.read(reinterpret_cast<char *>(&gs.mapMeta.isBaseCleared), sizeof(bool));
        file.read(reinterpret_cast<char *>(&gs.mapMeta.baseSuppliesLevel), sizeof(float));
        file.read(reinterpret_cast<char *>(&gs.mapMeta.activeVerminNests), sizeof(unsigned int));

        file.read(reinterpret_cast<char *>(&gs.titan.position), sizeof(Vector3D));
        file.read(reinterpret_cast<char *>(&gs.titan.health), sizeof(float));
        file.read(reinterpret_cast<char *>(&gs.titan.systems), sizeof(TitanComponents));

        file.close();
        std::cout << "[SAVE] Загружено из " << path << std::endl;
        return true;
    }

    bool SaveSystem::saveExists(unsigned int slot)
    {
        std::string path = "saves/slot_" + std::to_string(slot) + ".sav";
        return std::filesystem::exists(path);
    }

} // namespace bunker