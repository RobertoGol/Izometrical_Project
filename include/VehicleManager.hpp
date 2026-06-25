#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Collisions.hpp"
#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace bunker {

// ═══════════════════════════════════════════════════════
// Конфиг транспорта — загружается из vehicle.cfg
// Формат (ключ=значение):
//   name=Паромобиль
//   max_speed=6.5
//   acceleration=2.0
//   turn_speed=1.5
//   collision_radius=0.5
//   mass=heavy
//   drive_type=pressure    (pressure / throttle / hover)
//   max_pressure=240
//   texture=sprite.png
//   sound_engine=engine.wav
// ═══════════════════════════════════════════════════════
struct VehicleConfig {
    std::string id;                // Имя папки = ID (steamcar, motorcycle, helicopter...)
    std::string displayName = "UNKNOWN VEHICLE";
    float maxSpeed           = 6.5f;
    float acceleration       = 2.0f;   // lerp скорость разгона
    float deceleration       = 4.0f;   // lerp торможение
    float turnSpeed          = 1.5f;
    float collisionRadius    = 0.5f;
    float maxPressure        = 240.0f; // Для pressure-типа
    std::string driveType    = "throttle";  // pressure / throttle / hover
    std::string texturePath;
    std::string soundPath;
    bool loaded              = false;
};

// ═══════════════════════════════════════════════════════
// Живой экземпляр транспорта на карте
// Физика универсальная — параметры берутся из конфига
// ═══════════════════════════════════════════════════════
struct VehicleInstance {
    const VehicleConfig* config = nullptr;

    Vector3D position  = {0.0f, 0.0f, 0.0f};
    Vector3D velocity  = {0.0f, 0.0f, 0.0f};
    float    hullAngle = 0.0f;

    // Для pressure-типа (паромобиль)
    float currentPressure = 0.0f;

    // Для throttle-типа (мотоцикл и тп)
    float currentSpeed = 0.0f;

    bool isOccupied = false;  // Сидит ли в нём Пилот

    // ── Универсальная физика на основе конфига ──
    void updatePhysics(GameState& gs, const InputSnapshot& input, float dt) {
        if (!config) return;

        if (config->driveType == "pressure") {
            updatePressureDrive(gs, input, dt);
        } else if (config->driveType == "hover") {
            updateHoverDrive(gs, input, dt);
        } else {
            updateThrottleDrive(gs, input, dt);
        }
    }

private:
    // ── Тип "pressure" (SteamCar) — скорость от давления ──
    void updatePressureDrive(GameState& gs, const InputSnapshot& input, float dt) {
        if (input.moveForward > 0.0f) {
            currentPressure = std::min(config->maxPressure,
                                        currentPressure + 40.0f * dt);
        } else {
            currentPressure = std::max(0.0f, currentPressure - 60.0f * dt);
        }

        if (input.moveStrafe < 0.0f) hullAngle -= config->turnSpeed * dt;
        if (input.moveStrafe > 0.0f) hullAngle += config->turnSpeed * dt;

        float reverse = (input.moveForward < 0.0f) ? -3.0f : 0.0f;
        float targetSpd = (currentPressure / config->maxPressure) * config->maxSpeed + reverse;

        Vector3D targetVel = {
            std::cos(hullAngle) * targetSpd,
            std::sin(hullAngle) * targetSpd, 0.0f
        };

        velocity.x += (targetVel.x - velocity.x) * config->acceleration * dt;
        velocity.y += (targetVel.y - velocity.y) * config->acceleration * dt;

        applyMovement(gs, dt);
    }

    // ── Тип "throttle" (Motorcycle, Car...) — прямой газ ──
    void updateThrottleDrive(GameState& gs, const InputSnapshot& input, float dt) {
        bool moving = false;

        if (input.moveForward > 0.0f) {
            currentSpeed = std::min(config->maxSpeed, currentSpeed + 15.0f * dt);
            moving = true;
        }
        if (input.moveForward < 0.0f) {
            currentSpeed = std::max(-3.0f, currentSpeed - 20.0f * dt);
            moving = true;
        }

        if (input.moveStrafe < 0.0f) hullAngle -= config->turnSpeed * dt;
        if (input.moveStrafe > 0.0f) hullAngle += config->turnSpeed * dt;

        if (!moving)
            currentSpeed += (0.0f - currentSpeed) * config->deceleration * dt;

        Vector3D targetVel = {
            std::cos(hullAngle) * currentSpeed,
            std::sin(hullAngle) * currentSpeed, 0.0f
        };

        velocity.x += (targetVel.x - velocity.x) * config->acceleration * dt;
        velocity.y += (targetVel.y - velocity.y) * config->acceleration * dt;

        applyMovement(gs, dt);
    }

    // ── Тип "hover" (вертолёт, ховеркрафт) — свободное парение ──
    void updateHoverDrive(GameState& gs, const InputSnapshot& input, float dt) {
        Vector3D moveDir = {0.0f, 0.0f, 0.0f};

        // Изометрическое направление WASD
        if (input.moveForward > 0.0f) { moveDir.x += 1.0f; moveDir.y -= 1.0f; }
        if (input.moveForward < 0.0f) { moveDir.x -= 1.0f; moveDir.y += 1.0f; }
        if (input.moveStrafe  < 0.0f) { moveDir.x -= 1.0f; moveDir.y -= 1.0f; }
        if (input.moveStrafe  > 0.0f) { moveDir.x += 1.0f; moveDir.y += 1.0f; }

        float len = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
        Vector3D targetVel = {0.0f, 0.0f, 0.0f};

        if (len > 0.01f) {
            targetVel.x = (moveDir.x / len) * config->maxSpeed;
            targetVel.y = (moveDir.y / len) * config->maxSpeed;
        }

        velocity.x += (targetVel.x - velocity.x) * config->acceleration * dt;
        velocity.y += (targetVel.y - velocity.y) * config->acceleration * dt;

        applyMovement(gs, dt);
    }

    // ── Применяем движение с коллизиями ──
    void applyMovement(GameState& gs, float dt) {
        float nextX = position.x + velocity.x * dt;
        float nextY = position.y + velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, position.y, config->collisionRadius))
            position.x = nextX;
        if (!Collisions::checkWorldCollision(gs, position.x, nextY, config->collisionRadius))
            position.y = nextY;
    }
};

// ═══════════════════════════════════════════════════════
// Менеджер транспорта
// Сканирует папку assets/vehicles/*, загружает конфиги,
// управляет посадкой/высадкой
// ═══════════════════════════════════════════════════════
class VehicleManager {
private:
    std::vector<VehicleConfig>   m_Registry;     // Все зарегистрированные типы
    std::vector<VehicleInstance> m_Spawned;       // Все заспавненные на карте
    int m_ActiveVehicleIndex = -1;                // Индекс в m_Spawned, в котором сидит игрок

public:
    VehicleManager() = default;

    // ═══════════════════════════════════════════════
    // Сканирование папки assets/vehicles/
    // Каждая подпапка = один тип транспорта
    // Внутри должен быть vehicle.cfg
    // ═══════════════════════════════════════════════
    void scanAndLoadConfigs(const std::string& vehiclesDir = "assets/vehicles") {
        m_Registry.clear();

        if (!std::filesystem::exists(vehiclesDir)) {
            std::cout << "[VEHICLES] Папка " << vehiclesDir << " не найдена, пропускаем." << std::endl;
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(vehiclesDir)) {
            if (!entry.is_directory()) continue;

            std::string folderName = entry.path().filename().string();
            std::string cfgPath = entry.path().string() + "/vehicle.cfg";

            if (!std::filesystem::exists(cfgPath)) {
                std::cout << "[VEHICLES] Пропущена папка без vehicle.cfg: " << folderName << std::endl;
                continue;
            }

            VehicleConfig cfg;
            cfg.id = folderName;

            if (loadConfig(cfgPath, cfg)) {
                // Пути к ассетам относительно папки транспорта
                if (!cfg.texturePath.empty())
                    cfg.texturePath = entry.path().string() + "/" + cfg.texturePath;
                if (!cfg.soundPath.empty())
                    cfg.soundPath = entry.path().string() + "/" + cfg.soundPath;

                cfg.loaded = true;
                m_Registry.push_back(cfg);
                std::cout << "[VEHICLES] Зарегистрирован: " << cfg.displayName
                          << " [" << cfg.id << "] drive=" << cfg.driveType << std::endl;
            }
        }

        std::cout << "[VEHICLES] Всего загружено типов: " << m_Registry.size() << std::endl;
    }

    // ═══════════════════════════════════════════════
    // Спавн транспорта на карту по ID
    // ═══════════════════════════════════════════════
    bool spawnVehicle(const std::string& vehicleId, const Vector3D& pos) {
        const VehicleConfig* cfg = findConfig(vehicleId);
        if (!cfg) {
            std::cerr << "[VEHICLES] Неизвестный ID: " << vehicleId << std::endl;
            return false;
        }

        VehicleInstance inst;
        inst.config   = cfg;
        inst.position = pos;
        m_Spawned.push_back(inst);
        return true;
    }

    // ═══════════════════════════════════════════════
    // Посадка в ближайший транспорт (E)
    // ═══════════════════════════════════════════════
    bool mountNearest(GameState& gs) {
        if (gs.playerMode == UnitMode::Titan) return false;
        if (m_ActiveVehicleIndex >= 0) return false;

        float bestDist = 4.0f;  // Радиус взаимодействия
        int bestIdx = -1;

        for (size_t i = 0; i < m_Spawned.size(); ++i) {
            if (m_Spawned[i].isOccupied) continue;
            float dx = m_Spawned[i].position.x - gs.playerPos.x;
            float dy = m_Spawned[i].position.y - gs.playerPos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = static_cast<int>(i);
            }
        }

        if (bestIdx < 0) return false;

        m_ActiveVehicleIndex = bestIdx;
        m_Spawned[bestIdx].isOccupied = true;
        return true;
    }

    // ═══════════════════════════════════════════════
    // Высадка (X)
    // ═══════════════════════════════════════════════
    void unmount(GameState& gs) {
        if (m_ActiveVehicleIndex < 0) return;

        gs.playerPos = m_Spawned[m_ActiveVehicleIndex].position;
        gs.playerPos.x += 0.5f;

        m_Spawned[m_ActiveVehicleIndex].isOccupied = false;
        m_ActiveVehicleIndex = -1;
    }

    // ═══════════════════════════════════════════════
    // Обновление физики активного транспорта
    // ═══════════════════════════════════════════════
    void update(GameState& gs, const InputSnapshot& input, float dt) {
        if (m_ActiveVehicleIndex < 0) return;

        if (input.dismountVehicle) {
            unmount(gs);
            return;
        }

        auto& v = m_Spawned[m_ActiveVehicleIndex];
        v.updatePhysics(gs, input, dt);
        gs.playerPos = v.position;
    }

    // ═══════════════════════════════════════════════
    // Геттеры
    // ═══════════════════════════════════════════════
    bool isPlayerInVehicle() const { return m_ActiveVehicleIndex >= 0; }

    const VehicleConfig* getActiveConfig() const {
        if (m_ActiveVehicleIndex < 0) return nullptr;
        return m_Spawned[m_ActiveVehicleIndex].config;
    }

    const VehicleInstance* getActiveInstance() const {
        if (m_ActiveVehicleIndex < 0) return nullptr;
        return &m_Spawned[m_ActiveVehicleIndex];
    }

    const std::vector<VehicleConfig>& getRegistry()    const { return m_Registry; }
    const std::vector<VehicleInstance>& getSpawned()    const { return m_Spawned; }
    std::vector<VehicleInstance>& getSpawnedMut()              { return m_Spawned; }

private:
    // ── Парсер vehicle.cfg ──
    bool loadConfig(const std::string& path, VehicleConfig& cfg) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#' || line[0] == '/') continue;

            // Убираем \r
            if (!line.empty() && line.back() == '\r') line.pop_back();

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            if      (key == "name")             cfg.displayName     = val;
            else if (key == "max_speed")        cfg.maxSpeed        = std::stof(val);
            else if (key == "acceleration")     cfg.acceleration    = std::stof(val);
            else if (key == "deceleration")     cfg.deceleration    = std::stof(val);
            else if (key == "turn_speed")       cfg.turnSpeed       = std::stof(val);
            else if (key == "collision_radius") cfg.collisionRadius = std::stof(val);
            else if (key == "max_pressure")     cfg.maxPressure     = std::stof(val);
            else if (key == "drive_type")       cfg.driveType       = val;
            else if (key == "texture")          cfg.texturePath     = val;
            else if (key == "sound_engine")     cfg.soundPath       = val;
        }

        file.close();
        return true;
    }

    // ── Поиск конфига по ID ──
    const VehicleConfig* findConfig(const std::string& id) const {
        for (const auto& cfg : m_Registry) {
            if (cfg.id == id) return &cfg;
        }
        return nullptr;
    }
};

}  // namespace bunker
