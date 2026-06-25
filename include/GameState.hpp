#pragma once

#include "Types.hpp"
#include "Constants.hpp"
#include <vector>
#include <array>

namespace bunker {

// Единый объект состояния игры — никаких глобальных extern!
// Передаётся по ссылке во все системы.

struct GameState {
    // ── Игрок ──
    UnitMode   playerMode      = UnitMode::Scout;
    Vector3D   playerPos       = {5.0f, 5.0f, 0.0f};
    float      playerHealth    = Config::PLAYER_START_HP;
    float      playerMaxHealth = Config::PLAYER_START_HP;
    float      playerSpeed     = Config::PLAYER_WALK_SPEED;
    float      fireCooldown    = 0.0f;
    float      playerErosionLevel = 0.0f;
    int        score           = 0;

    // ── Прицеливание ──
    bool       isAiming        = false;
    Vector3D   mouseWorldPos   = {0.0f, 0.0f, 0.0f};

    // ── Камера ──
    Vector3D   cameraTarget    = {5.0f, 5.0f, 0.0f};
    float      cameraZoom      = Config::CAMERA_BASE_ZOOM;

    // ── Баллистика ──
    std::vector<Bullet> bullets;

    // ── Враги ──
    std::vector<Enemy>  enemies;
    float enemySpawnTimer = 0.0f;

    // ── Титан / БТ-7274 ──
    TitanAlly  titan;

    // ── Регион / Убежище ──
    Vault17GridState   regionalGrid;
    Vault17Progression bunkerProgression;
    Vector3D           towerPosition = {Config::TOWER_X, Config::TOWER_Y, 0.0f};

    // ── Карта (тайлы 20x20) ──
    // 0 = пол, 1 = стена, 2 = вода, 3 = эрозия
    std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH>   sectorMap{};
    std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH>   wallDurability{};
    std::array<std::array<float, Config::MAP_HEIGHT>, Config::MAP_WIDTH> etherErosionMap{};

    // ── Лут-контейнеры на карте ──
    std::vector<LootContainer> lootContainers;

    // ── Метаданные сессии карты ──
    SessionMapMetaData mapMeta;

    // ── Прогрессия персонажа ──
    CharacterProgression characterProg;
    StoryState           story;

    // ── Флаг работы игры ──
    bool isRunning = true;

    // ── DeltaTime текущего кадра ──
    float deltaTime = 0.0f;

    // ── Размер окна ──
    float windowWidth  = static_cast<float>(Config::SCREEN_WIDTH);
    float windowHeight = static_cast<float>(Config::SCREEN_HEIGHT);
};

}  // namespace bunker
