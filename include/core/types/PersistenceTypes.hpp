#pragma once

#include "core/Constants.hpp"
#include "core/ECS.hpp"
#include "core/types/Enums.hpp"
#include "core/types/GameplayTypes.hpp"
#include "core/types/Vectors.hpp"
#include <string>

namespace bunker
{
    inline constexpr unsigned int SAVE_FORMAT_VERSION = 19;

    // ══════════════════════════════════════════════════════════════════════
    // Данные сохранения
    // ══════════════════════════════════════════════════════════════════════

    struct SaveFileHeader
    {
        char magic[4] = {'B', 'S', 'A', 'V'};
        unsigned int version = SAVE_FORMAT_VERSION; // BSAV v19
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

    struct RuntimeWorldSaveData
    {
        int terrainSeed = 0;
        float terrainSize = 100.0f;
        float terrainHeightScale = 3.0f;
        float terrainFrequency = 0.15f;
        std::uint32_t ecsEntityCount = 0;
        std::uint32_t campObjectCount = 0;
        std::uint32_t breakableStateCount = 0;
    };

    struct RenderEntitySaveData
    {
        EntityID entity = MAX_ENTITIES;
        TransformComponent transform;
        std::uint32_t materialID = 0;
    };

    struct CampObjectSaveData
    {
        int id = 0;
        std::uint32_t type = 0;
        int tileX = 0;
        int tileY = 0;
        float health = 100.0f;
    };

    struct BreakableSaveData
    {
        int id = 0;
        std::uint32_t kind = 0;
        Vector3D position;
        float health = 30.0f;
        float radius = 0.35f;
        bool broken = false;
        Vector3D velocity;
    };

} // namespace bunker
