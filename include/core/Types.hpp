#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include "Constants.hpp"

namespace bunker
{

    // ══════════════════════════════════════════════════════════════════════
    // Перечисления
    // ══════════════════════════════════════════════════════════════════════

    enum class AIState
    {
        Follow,
        Guard,
        Combat
    };
    enum class TankWeaponMode
    {
        Cannon,
        AutoCannon
    };
    enum class MissileStrikeMode
    {
        Ballistic,
        Artillery
    };
    enum class BulletType
    {
        Standard,
        Pellet,
        BallisticMissile,
        ArtilleryMissile
    };
    enum class UnitMode
    {
        Scout,
        Titan
    };

    enum class PilotClass
    {
        Grapple,
        Cloak,
        Stim,
        PhaseShift,
        HoloPilot,
        AWall,
        PulseBlade
    };

    enum class TitanClass
    {
        Ion,
        Scorch,
        Northstar,
        Ronin,
        Tone,
        Legion,
        Monarch
    };

    enum class ItemType
    {
        Weapon,
        Ammo,
        Medicine,
        Resource,
        Quest
    };
    enum class CampObjectType
    {
        ConcreteWall,
        DefenseTurret,
        SupplyCrate
    };

    enum class LootContainerType
    {
        WoodenCrate,
        IronSafe,
        DevVault
    };

    enum class VehicleType
    {
        None,
        SteamCar,
        Motorcycle
    };

    // ══════════════════════════════════════════════════════════════════════
    // Математические примитивы
    // ══════════════════════════════════════════════════════════════════════

    struct Vector2D
    {
        float x = 0.0f;
        float y = 0.0f;

        Vector2D() = default;
        Vector2D(float ax, float ay) : x(ax), y(ay) {}

        Vector2D operator+(const Vector2D &o) const { return {x + o.x, y + o.y}; }
        Vector2D operator-(const Vector2D &o) const { return {x - o.x, y - o.y}; }
        Vector2D operator*(float s) const { return {x * s, y * s}; }
        Vector2D &operator+=(const Vector2D &o)
        {
            x += o.x;
            y += o.y;
            return *this;
        }
        Vector2D &operator-=(const Vector2D &o)
        {
            x -= o.x;
            y -= o.y;
            return *this;
        }

        float length() const { return std::sqrt(x * x + y * y); }
        float lengthSq() const { return x * x + y * y; }

        Vector2D normalized() const
        {
            float len = length();
            if (len < 0.0001f)
                return {0.0f, 0.0f};
            return {x / len, y / len};
        }
    };

    struct Vector3D
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        Vector3D() = default;
        Vector3D(float ax, float ay, float az = 0.0f) : x(ax), y(ay), z(az) {}

        Vector3D operator+(const Vector3D &o) const { return {x + o.x, y + o.y, z + o.z}; }
        Vector3D operator-(const Vector3D &o) const { return {x - o.x, y - o.y, z - o.z}; }
        Vector3D operator*(float s) const { return {x * s, y * s, z * s}; }
        Vector3D &operator+=(const Vector3D &o)
        {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }

        float length() const { return std::sqrt(x * x + y * y + z * z); }
        float lengthSq() const { return x * x + y * y + z * z; }

        Vector2D xy() const { return {x, y}; }

        Vector3D normalized() const
        {
            float len = length();
            if (len < 0.0001f)
                return {};
            return {x / len, y / len, z / len};
        }
    };

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
        float physicsRadius = 0.35f;
        std::vector<InventoryItem> containsItems;
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

    // ══════════════════════════════════════════════════════════════════════
    // Рендер-очередь (для Z-сортировки)
    // ══════════════════════════════════════════════════════════════════════

    struct RenderObject
    {
        float depth;
        std::function<void()> drawFunc;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Прогрессия персонажа (из GMyGameDoNotTouch)
    // ══════════════════════════════════════════════════════════════════════

    struct CharacterProgression
    {
        int level = 1;
        int experience = 0;
        int unusedPoints = 0;
        float hp = Config::PLAYER_START_HP;
        float maxHp = Config::PLAYER_START_HP;
        float mp = 50.0f;
        float maxMp = 50.0f;
        float sanityLine = 100.0f; // Линия Разума (Резервный предохранитель сознания)
        float soulLine = 100.0f;   // Линия Души (Система безопасности воспоминаний)
        std::vector<InventoryItem> inventory;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Сюжет
    // ══════════════════════════════════════════════════════════════════════

    struct StoryState
    {
        bool pipPadRecovered = false;
        bool archiveRecovered = false;
        bool tankLinked = false;
        bool bucketRecovered = false;
        bool exitedBunker = false;
    };

    struct StoryRouteEntry
    {
        std::string text;
        bool completed = false;
    };

} // namespace bunker