#pragma once

#include "Types.hpp"
#include "Constants.hpp"
#include <vector>
#include <array>

namespace bunker
{

    class HostileAISystem;

    // ═══════════════════════════════════════════════════════════════════════
    // Доменные стейт-контроллеры движка (TARGET_ARCHITECTURE.md §7)
    // ═══════════════════════════════════════════════════════════════════════

    // ── 1. Домен состояния Пилота ──
    struct PlayerRuntimeState
    {
        UnitMode playerMode = UnitMode::Scout;
        Vector3D playerPos = {5.0f, 5.0f, 0.0f};
        float playerHealth = Config::PLAYER_START_HP;
        float playerMaxHealth = Config::PLAYER_START_HP;
        float playerSpeed = Config::PLAYER_WALK_SPEED;
        float fireCooldown = 0.0f;
        float playerErosionLevel = 0.0f;
        float worldVisibilityModifier = 1.0f;
        int score = 0;
    };

    // ── 2. Домен тайловой сетки Убежища 17 ──
    struct WorldGridState
    {
        // 0 = пол, 1 = стена, 2 = вода, 3 = эрозия
        std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH> sectorMap{};
        std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH> wallDurability{};
        std::array<std::array<float, Config::MAP_HEIGHT>, Config::MAP_WIDTH> etherErosionMap{};
        Vault17GridState regionalGrid;
        Vector3D towerPosition = {Config::TOWER_X, Config::TOWER_Y, 0.0f};
    };

    // ── 3. Домен сессионных данных и прогрессии ──
    struct SessionPersistenceData
    {
        std::vector<LootContainer> lootContainers;
        SessionMapMetaData mapMeta;
        Vault17Progression bunkerProgression;
        CharacterProgression characterProg;
        StoryState story;
        bool isRunning = true;
        float deltaTime = 0.0f;
        float windowWidth = static_cast<float>(Config::SCREEN_WIDTH);
        float windowHeight = static_cast<float>(Config::SCREEN_HEIGHT);
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Единый агрегированный объект состояния игры
    // Наследует домены для 100% обратной совместимости со старым кодом!
    // ═══════════════════════════════════════════════════════════════════════

    struct GameState : public PlayerRuntimeState, public WorldGridState, public SessionPersistenceData
    {
        HostileAISystem *hostileAI = nullptr;

        // ── Прицеливание ──
        bool isAiming = false;
        Vector3D mouseWorldPos = {0.0f, 0.0f, 0.0f};

        // ── Камера ──
        Vector3D cameraTarget = {5.0f, 5.0f, 0.0f};
        float cameraZoom = Config::CAMERA_BASE_ZOOM;

        // ── Баллистика ──
        std::vector<Bullet> bullets;

        // ── Враги ──
        std::vector<Enemy> enemies;
        float enemySpawnTimer = 0.0f;

        // ── Титан / БТ-7274 ──
        TitanAlly titan;
    };

} // namespace bunker