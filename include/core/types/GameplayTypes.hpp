#pragma once

#include "core/Constants.hpp"
#include "core/types/Enums.hpp"
#include "core/types/Vectors.hpp"
#include <string>
#include <vector>

namespace bunker
{
    // ══════════════════════════════════════════════════════════════════════
    // Игровые структуры — Баллистика
    // ══════════════════════════════════════════════════════════════════════

    struct Bullet
    {
        Vector3D start;
        Vector3D current;
        Vector3D direction;
        float speed = Config::BULLET_SPEED;
        float distanceTraveled = 0.0f;
        float maxDistance = Config::BULLET_MAX_RANGE;
        bool isAlive = true;
        BulletType type = BulletType::Standard;
        float splashRadius = 0.0f;
        Vector3D targetPos; // Для артиллерийских снарядов
    };

    // ══════════════════════════════════════════════════════════════════════
    // Враги
    // ══════════════════════════════════════════════════════════════════════

    struct Enemy
    {
        Vector3D position;
        float health = Config::ENEMY_BASE_HP;
        float speed = Config::ENEMY_BASE_SPEED;
        bool isAlive = true;
        float radius = Config::ENEMY_RADIUS;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Титан / Танк
    // ══════════════════════════════════════════════════════════════════════

    struct TitanComponents
    {
        float coreEnergy = 100.0f;
        float tracksCondition = 100.0f;
        float turretStatus = 100.0f;
        float sensorLink = 100.0f;
    };

    struct TitanAlly
    {
        Vector3D position = {2.0f, 2.0f, 0.0f};
        float health = Config::TITAN_HP;
        float maxHealth = Config::TITAN_HP;
        float speed = Config::TITAN_SPEED;
        float fireCooldown = 0.0f;
        bool isPiloted = false;
        AIState aiState = AIState::Follow;
        TitanComponents systems;
        bool hasMissileModule = true;
        TankWeaponMode currentWeapon = TankWeaponMode::Cannon;
        MissileStrikeMode missileMode = MissileStrikeMode::Ballistic;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Состояние сетки региона / Убежища
    // ══════════════════════════════════════════════════════════════════════

    struct Vault17GridState
    {
        bool towerSyncRecovered = true;
        bool localRelayAvailable = true;
        bool feyRingGateUnlocked = true;
        float towerHealth = 200.0f;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Модификаторы характеристик
    // ══════════════════════════════════════════════════════════════════════

    struct StatModifiers
    {
        float moveSpeed = Config::PLAYER_WALK_SPEED;
        float maxHealth = Config::PLAYER_START_HP;
        float damageMultiplier = 1.0f;
        float erosionResistance = 0.0f;
        bool isVehicleMode = false;
        std::string weaponLabel = "STANDARD CARBINE";
    };

    // ══════════════════════════════════════════════════════════════════════
    // Щиты и Lock-On (Титан)
    // ══════════════════════════════════════════════════════════════════════

    struct VortexShieldState
    {
        bool isActive = false;
        float energy = 100.0f;
        int caughtBulletsCount = 0;
    };

    struct LockOnTarget
    {
        int enemyIndex = -1;
        int locksCount = 0;
        bool isFullyLocked = false;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Прогрессия / Pip-Pad
    // ══════════════════════════════════════════════════════════════════════

    struct Vault17Progression
    {
        bool hasFoundPipPad = false;
        Vector3D pipPadSpawnPos = {Config::PIPPAD_SPAWN_X, Config::PIPPAD_SPAWN_Y, 0.0f};
    };

    // ══════════════════════════════════════════════════════════════════════
    // Инвентарь
    // ══════════════════════════════════════════════════════════════════════

    struct InventoryItem
    {
        unsigned int itemID = 0;
        ItemType type = ItemType::Resource;
        int quantity = 1;
        float weightPerUnit = 0.1f;
        std::string displayName = "UNKNOWN ITEM";
    };

    // ══════════════════════════════════════════════════════════════════════
    // Лут-контейнеры
    // ══════════════════════════════════════════════════════════════════════

    struct LootContainer
    {
        Vector3D position;
        LootContainerType type = LootContainerType::WoodenCrate;
        bool isOpened = false;
        float respawnDelaySeconds = 0.0f;
        float respawnTimerSeconds = 0.0f;
        float physicsRadius = 0.35f;
        std::vector<InventoryItem> containsItems;
    };

    struct LoosePickup
    {
        Vector3D position;
        InventoryItem item;
        bool collected = false;
        float interactionRadius = 1.0f;
    };

    struct NeutralNpc
    {
        int id = 0;
        Vector3D position;
        std::string displayName = "Vault Resident";
        bool hasTalked = false;
        float interactionRadius = 1.4f;
    };

    // ══════════════════════════════════════════════════════════════════════
    // C.A.M.P. строительство
    // ══════════════════════════════════════════════════════════════════════

    struct CampPreview
    {
        int tileX = 0;
        int tileY = 0;
        CampObjectType activeType = CampObjectType::ConcreteWall;
        bool isPlacementValid = false;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Крюк-кошка (Grapple)
    // ══════════════════════════════════════════════════════════════════════

    struct GrapplePhysics
    {
        Vector3D hookPoint;
        float length = 0.0f;
        Vector3D velocity;
        bool isAttached = false;
        bool isActive = false;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Защитный экран A-Wall
    // ══════════════════════════════════════════════════════════════════════

    struct DeployableShield
    {
        Vector3D position;
        float health = 150.0f;
        float currentHealth = 500.0f;
        float lifetime = 5.0f;
        bool isDeployed = false;
    };


} // namespace bunker
