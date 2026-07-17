#pragma once

#include "core/Constants.hpp"
#include "core/types/Enums.hpp"
#include "core/types/GameplayTypes.hpp"
#include "core/types/Vectors.hpp"
#include <string>

namespace bunker
{
    // ══════════════════════════════════════════════════════════════════════
    // Данные сохранения
    // ══════════════════════════════════════════════════════════════════════

    struct SaveFileHeader
    {
        char magic[4] = {'B', 'S', 'A', 'V'};
        unsigned int version = 17; // BSAV v17 (Конгресс ИИ — без CRC32)
        unsigned int slotIndex = 0;
    };

    struct PlayerSaveData
    {
        Vector3D position;
        UnitMode currentMode = UnitMode::Scout;
        float health = Config::PLAYER_START_HP;
        float maxHealth = Config::PLAYER_START_HP;
        float erosionLevel = 0.0f;
        int currentScore = 0;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Метаданные карты сессии
    // ══════════════════════════════════════════════════════════════════════

    struct SessionMapMetaData
    {
        std::string currentMapName = "base";
        bool isBaseCleared = false;
        float baseSuppliesLevel = 100.0f;
        unsigned int activeVerminNests = 0;
    };


} // namespace bunker
