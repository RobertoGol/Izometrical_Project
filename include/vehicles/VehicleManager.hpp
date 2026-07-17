#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "entities/Collisions.hpp"
#include "core/Constants.hpp"
#include "engine/InputManager.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace bunker
{

    struct VehicleConfig
    {
        std::string id;
        std::string displayName = "UNKNOWN VEHICLE";
        float maxSpeed = 6.5f;
        float acceleration = 2.0f;
        float deceleration = 4.0f;
        float turnSpeed = 1.5f;
        float collisionRadius = 0.5f;
        float maxPressure = 240.0f;
        std::string driveType = "throttle";
        std::string texturePath;
        std::string soundPath;
        bool loaded = false;
    };

    struct VehicleInstance
    {
        const VehicleConfig *config = nullptr;
        Vector3D position = {0.0f, 0.0f, 0.0f};
        Vector3D velocity = {0.0f, 0.0f, 0.0f};
        float hullAngle = 0.0f;
        float currentPressure = 0.0f;
        float currentSpeed = 0.0f;
        bool isOccupied = false;

        void updatePhysics(GameState &gs, const InputSnapshot &input, float dt);

    private:
        void updatePressureDrive(GameState &gs, const InputSnapshot &input, float dt);
        void updateThrottleDrive(GameState &gs, const InputSnapshot &input, float dt);
        void updateHoverDrive(GameState &gs, const InputSnapshot &input, float dt);
        void applyMovement(GameState &gs, float dt);
    };

    class VehicleManager
    {
    private:
        std::vector<VehicleConfig> m_Registry;
        std::vector<VehicleInstance> m_Spawned;
        int m_ActiveVehicleIndex = -1;

    public:
        VehicleManager() = default;

        void scanAndLoadConfigs(const std::string &vehiclesDir = "assets/vehicles");
        bool spawnVehicle(const std::string &configId, const Vector3D &pos);
        bool mountNearest(GameState &gs);
        void dismount(GameState &gs);
        void update(GameState &gs, const InputSnapshot &input, float dt);

        bool isPlayerInVehicle() const { return m_ActiveVehicleIndex >= 0; }
        float getCarPressurePercent() const;
        const VehicleConfig *getActiveConfig() const;
        const VehicleInstance *getActiveInstance() const;
        const std::vector<VehicleConfig> &getRegistry() const { return m_Registry; }
        const std::vector<VehicleInstance> &getSpawned() const { return m_Spawned; }
        std::vector<VehicleInstance> &getSpawnedMut() { return m_Spawned; }

    private:
        bool loadConfig(const std::string &path, VehicleConfig &cfg);
        const VehicleConfig *findConfig(const std::string &id) const;
    };

} // namespace bunker