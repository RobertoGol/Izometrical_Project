#include "vehicles/VehicleManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cmath>
#include <algorithm>

namespace bunker
{

    void VehicleInstance::updatePhysics(GameState &gs, const InputSnapshot &input, float dt)
    {
        if (!config)
            return;

        if (config->driveType == "pressure")
        {
            updatePressureDrive(gs, input, dt);
        }
        else if (config->driveType == "hover")
        {
            updateHoverDrive(gs, input, dt);
        }
        else
        {
            updateThrottleDrive(gs, input, dt);
        }
    }

    void VehicleInstance::updatePressureDrive(GameState &gs, const InputSnapshot &input, float dt)
    {
        if (input.moveForward > 0.0f)
        {
            currentPressure = std::min(config->maxPressure, currentPressure + 40.0f * dt);
        }
        else
        {
            currentPressure = std::max(0.0f, currentPressure - 60.0f * dt);
        }

        if (input.moveStrafe < 0.0f)
            hullAngle -= config->turnSpeed * dt;
        if (input.moveStrafe > 0.0f)
            hullAngle += config->turnSpeed * dt;

        float reverse = (input.moveForward < 0.0f) ? -3.0f : 0.0f;
        float targetSpd = (currentPressure / config->maxPressure) * config->maxSpeed + reverse;

        Vector3D targetVel = {
            std::cos(hullAngle) * targetSpd,
            std::sin(hullAngle) * targetSpd, 0.0f};

        velocity.x += (targetVel.x - velocity.x) * config->acceleration * dt;
        velocity.y += (targetVel.y - velocity.y) * config->acceleration * dt;

        applyMovement(gs, dt);
    }

    void VehicleInstance::updateThrottleDrive(GameState &gs, const InputSnapshot &input, float dt)
    {
        bool moving = false;

        if (input.moveForward > 0.0f)
        {
            currentSpeed = std::min(config->maxSpeed, currentSpeed + config->acceleration * dt);
            moving = true;
        }
        else if (input.moveForward < 0.0f)
        {
            currentSpeed = std::max(-config->maxSpeed * 0.4f, currentSpeed - config->deceleration * dt);
            moving = true;
        }
        else
        {
            currentSpeed *= std::pow(0.05f, dt);
        }

        if (std::abs(currentSpeed) > 0.1f)
        {
            float turnDir = (currentSpeed > 0.0f) ? 1.0f : -1.0f;
            if (input.moveStrafe < 0.0f)
                hullAngle -= config->turnSpeed * turnDir * dt;
            if (input.moveStrafe > 0.0f)
                hullAngle += config->turnSpeed * turnDir * dt;
        }

        velocity.x = std::cos(hullAngle) * currentSpeed;
        velocity.y = std::sin(hullAngle) * currentSpeed;

        applyMovement(gs, dt);
    }

    void VehicleInstance::updateHoverDrive(GameState &gs, const InputSnapshot &input, float dt)
    {
        Vector3D inputVector = {
            std::cos(hullAngle) * input.moveForward - std::sin(hullAngle) * input.moveStrafe,
            std::sin(hullAngle) * input.moveForward + std::cos(hullAngle) * input.moveStrafe,
            0.0f};

        if (input.moveForward != 0.0f || input.moveStrafe != 0.0f)
        {
            float len = std::sqrt(inputVector.x * inputVector.x + inputVector.y * inputVector.y);
            if (len > 0.0001f)
            {
                inputVector.x /= len;
                inputVector.y /= len;
            }

            velocity.x += inputVector.x * config->acceleration * dt;
            velocity.y += inputVector.y * config->acceleration * dt;

            float spd = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            if (spd > config->maxSpeed)
            {
                velocity.x = (velocity.x / spd) * config->maxSpeed;
                velocity.y = (velocity.y / spd) * config->maxSpeed;
            }

            if (input.moveStrafe != 0.0f)
            {
                hullAngle += input.moveStrafe * config->turnSpeed * 0.5f * dt;
            }
        }
        else
        {
            velocity.x *= std::pow(0.1f, dt);
            velocity.y *= std::pow(0.1f, dt);
        }

        position.z = 0.5f + std::sin(static_cast<float>(rand() % 100)) * 0.05f;
        applyMovement(gs, dt);
    }

    void VehicleInstance::applyMovement(GameState &gs, float dt)
    {
        float nextX = position.x + velocity.x * dt;
        float nextY = position.y + velocity.y * dt;

        float r = config ? config->collisionRadius : 0.5f;

        if (!Collisions::checkWorldCollision(gs, nextX, position.y, r))
        {
            position.x = nextX;
        }
        else
        {
            velocity.x *= -0.3f;
        }

        if (!Collisions::checkWorldCollision(gs, position.x, nextY, r))
        {
            position.y = nextY;
        }
        else
        {
            velocity.y *= -0.3f;
        }
    }

    void VehicleManager::scanAndLoadConfigs(const std::string &vehiclesDir)
    {
        m_Registry.clear();

        if (!std::filesystem::exists(vehiclesDir))
        {
            std::cout << "[VEHICLES] Папка " << vehiclesDir << " не найдена, пропускаем." << std::endl;
            return;
        }

        for (const auto &entry : std::filesystem::directory_iterator(vehiclesDir))
        {
            if (!entry.is_directory())
                continue;

            std::string folderName = entry.path().filename().string();
            std::string cfgPath = entry.path().string() + "/vehicle.cfg";

            if (!std::filesystem::exists(cfgPath))
                continue;

            VehicleConfig cfg;
            cfg.id = folderName;

            if (loadConfig(cfgPath, cfg))
            {
                if (!cfg.texturePath.empty())
                    cfg.texturePath = entry.path().string() + "/" + cfg.texturePath;
                if (!cfg.soundPath.empty())
                    cfg.soundPath = entry.path().string() + "/" + cfg.soundPath;

                cfg.loaded = true;
                m_Registry.push_back(cfg);
                std::cout << "[VEHICLES] Зарегистрирован: " << cfg.displayName
                          << " [" << cfg.id << "] spd=" << cfg.maxSpeed << std::endl;
            }
        }

        std::cout << "[VEHICLES] Всего типов транспорта: " << m_Registry.size() << std::endl;
    }

    bool VehicleManager::spawnVehicle(const std::string &configId, const Vector3D &pos)
    {
        const VehicleConfig *cfg = findConfig(configId);
        if (!cfg)
        {
            std::cerr << "[VEHICLES] Конфиг '" << configId << "' не найден!" << std::endl;
            return false;
        }

        VehicleInstance inst;
        inst.config = cfg;
        inst.position = pos;
        m_Spawned.push_back(inst);

        std::cout << "[VEHICLES] Спавн транспорта: " << cfg->displayName
                  << " в [" << pos.x << ", " << pos.y << "]" << std::endl;
        return true;
    }

    bool VehicleManager::mountNearest(GameState &gs)
    {
        if (gs.playerMode == UnitMode::Titan)
            return false;
        if (m_ActiveVehicleIndex >= 0)
            return false;

        float nearestDistSq = 4.0f;
        int bestIndex = -1;

        for (int i = 0; i < static_cast<int>(m_Spawned.size()); ++i)
        {
            float dx = m_Spawned[i].position.x - gs.playerPos.x;
            float dy = m_Spawned[i].position.y - gs.playerPos.y;
            float distSq = dx * dx + dy * dy;

            if (distSq < nearestDistSq)
            {
                nearestDistSq = distSq;
                bestIndex = i;
            }
        }

        if (bestIndex >= 0)
        {
            m_ActiveVehicleIndex = bestIndex;
            m_Spawned[bestIndex].isOccupied = true;
            gs.playerPos = m_Spawned[bestIndex].position;

            std::cout << "[VEHICLES] Посадка в " << m_Spawned[bestIndex].config->displayName << std::endl;
            return true;
        }

        return false;
    }

    void VehicleManager::dismount(GameState &gs)
    {
        if (m_ActiveVehicleIndex < 0)
            return;

        VehicleInstance &v = m_Spawned[m_ActiveVehicleIndex];
        v.isOccupied = false;

        gs.playerPos.x = v.position.x + 1.0f;
        gs.playerPos.y = v.position.y;
        gs.playerPos.z = 0.0f;

        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS))
        {
            gs.playerPos.x = v.position.x - 1.0f;
        }

        std::cout << "[VEHICLES] Выход из " << v.config->displayName << std::endl;
        m_ActiveVehicleIndex = -1;
    }

    void VehicleManager::update(GameState &gs, const InputSnapshot &input, float dt)
    {
        if (m_ActiveVehicleIndex < 0)
            return;

        VehicleInstance &v = m_Spawned[m_ActiveVehicleIndex];
        v.updatePhysics(gs, input, dt);
        gs.playerPos = v.position;
    }

    float VehicleManager::getCarPressurePercent() const
    {
        if (m_ActiveVehicleIndex < 0)
            return 0.0f;
        const auto &inst = m_Spawned[m_ActiveVehicleIndex];
        if (!inst.config || inst.config->maxPressure <= 0.0f)
            return 0.0f;
        return inst.currentPressure / inst.config->maxPressure;
    }

    const VehicleConfig *VehicleManager::getActiveConfig() const
    {
        if (m_ActiveVehicleIndex < 0)
            return nullptr;
        return m_Spawned[m_ActiveVehicleIndex].config;
    }

    const VehicleInstance *VehicleManager::getActiveInstance() const
    {
        if (m_ActiveVehicleIndex < 0)
            return nullptr;
        return &m_Spawned[m_ActiveVehicleIndex];
    }

    bool VehicleManager::loadConfig(const std::string &path, VehicleConfig &cfg)
    {
        std::ifstream file(path);
        if (!file.is_open())
            return false;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#' || line[0] == '/')
                continue;

            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            auto eq = line.find('=');
            if (eq == std::string::npos)
                continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            if (key == "name")
                cfg.displayName = val;
            else if (key == "max_speed")
                cfg.maxSpeed = std::stof(val);
            else if (key == "acceleration")
                cfg.acceleration = std::stof(val);
            else if (key == "deceleration")
                cfg.deceleration = std::stof(val);
            else if (key == "turn_speed")
                cfg.turnSpeed = std::stof(val);
            else if (key == "collision_radius")
                cfg.collisionRadius = std::stof(val);
            else if (key == "max_pressure")
                cfg.maxPressure = std::stof(val);
            else if (key == "drive_type")
                cfg.driveType = val;
            else if (key == "texture")
                cfg.texturePath = val;
            else if (key == "sound_engine")
                cfg.soundPath = val;
        }

        file.close();
        return true;
    }

    const VehicleConfig *VehicleManager::findConfig(const std::string &id) const
    {
        for (const auto &cfg : m_Registry)
        {
            if (cfg.id == id)
                return &cfg;
        }
        return nullptr;
    }

} // namespace bunker
