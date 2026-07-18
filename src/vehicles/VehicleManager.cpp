#include "vehicles/VehicleManager.hpp"
#include "engine/Log.hpp"
#include <filesystem>
#include <fstream>
#include <map>

namespace bunker
{
    namespace
    {
        using VehicleConfigSetter = void (*)(VehicleConfig&, const std::string&);

        const std::map<std::string, VehicleConfigSetter>& vehicleConfigSetters()
        {
            static const std::map<std::string, VehicleConfigSetter> setters = {
                {"name", [](VehicleConfig& cfg, const std::string& val) { cfg.displayName = val; }},
                {"max_speed", [](VehicleConfig& cfg, const std::string& val) { cfg.maxSpeed = std::stof(val); }},
                {"acceleration", [](VehicleConfig& cfg, const std::string& val) { cfg.acceleration = std::stof(val); }},
                {"deceleration", [](VehicleConfig& cfg, const std::string& val) { cfg.deceleration = std::stof(val); }},
                {"turn_speed", [](VehicleConfig& cfg, const std::string& val) { cfg.turnSpeed = std::stof(val); }},
                {"collision_radius",
                 [](VehicleConfig& cfg, const std::string& val) { cfg.collisionRadius = std::stof(val); }},
                {"max_pressure", [](VehicleConfig& cfg, const std::string& val) { cfg.maxPressure = std::stof(val); }},
                {"drive_type", [](VehicleConfig& cfg, const std::string& val) { cfg.driveType = val; }},
                {"texture", [](VehicleConfig& cfg, const std::string& val) { cfg.texturePath = val; }},
                {"sound_engine", [](VehicleConfig& cfg, const std::string& val) { cfg.soundPath = val; }},
            };
            return setters;
        }
    } // namespace

    void VehicleManager::scanAndLoadConfigs(const std::string& vehiclesDir)
    {
        m_Registry.clear();

        if (!std::filesystem::exists(vehiclesDir))
        {
            bunker::logInfo() << "[VEHICLES] Папка " << vehiclesDir << " не найдена, пропускаем." << std::endl;
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(vehiclesDir))
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
                bunker::logInfo() << "[VEHICLES] Зарегистрирован: " << cfg.displayName << " [" << cfg.id
                                  << "] spd=" << cfg.maxSpeed << std::endl;
            }
        }

        bunker::logInfo() << "[VEHICLES] Всего типов транспорта: " << m_Registry.size() << std::endl;
    }

    bool VehicleManager::spawnVehicle(const std::string& configId, const Vector3D& pos)
    {
        const VehicleConfig* cfg = findConfig(configId);
        if (!cfg)
        {
            bunker::logError() << "[VEHICLES] Конфиг '" << configId << "' не найден!" << std::endl;
            return false;
        }

        VehicleInstance inst;
        inst.config = cfg;
        inst.position = pos;
        m_Spawned.push_back(inst);

        bunker::logInfo() << "[VEHICLES] Спавн транспорта: " << cfg->displayName << " в [" << pos.x << ", " << pos.y
                          << "]" << std::endl;
        return true;
    }

    bool VehicleManager::mountNearest(GameState& gs)
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

            bunker::logInfo() << "[VEHICLES] Посадка в " << m_Spawned[bestIndex].config->displayName << std::endl;
            return true;
        }

        return false;
    }

    void VehicleManager::dismount(GameState& gs)
    {
        if (m_ActiveVehicleIndex < 0)
            return;

        VehicleInstance& v = m_Spawned[m_ActiveVehicleIndex];
        v.isOccupied = false;

        gs.playerPos.x = v.position.x + 1.0f;
        gs.playerPos.y = v.position.y;
        gs.playerPos.z = 0.0f;

        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS))
        {
            gs.playerPos.x = v.position.x - 1.0f;
        }

        bunker::logInfo() << "[VEHICLES] Выход из " << v.config->displayName << std::endl;
        m_ActiveVehicleIndex = -1;
    }

    void VehicleManager::update(GameState& gs, const InputSnapshot& input, float dt)
    {
        if (m_ActiveVehicleIndex < 0)
            return;

        VehicleInstance& v = m_Spawned[m_ActiveVehicleIndex];
        v.updatePhysics(gs, input, dt);
        gs.playerPos = v.position;
    }

    float VehicleManager::getCarPressurePercent() const
    {
        if (m_ActiveVehicleIndex < 0)
            return 0.0f;
        const auto& inst = m_Spawned[m_ActiveVehicleIndex];
        if (!inst.config || inst.config->maxPressure <= 0.0f)
            return 0.0f;
        return inst.currentPressure / inst.config->maxPressure;
    }

    const VehicleConfig* VehicleManager::getActiveConfig() const
    {
        if (m_ActiveVehicleIndex < 0)
            return nullptr;
        return m_Spawned[m_ActiveVehicleIndex].config;
    }

    const VehicleInstance* VehicleManager::getActiveInstance() const
    {
        if (m_ActiveVehicleIndex < 0)
            return nullptr;
        return &m_Spawned[m_ActiveVehicleIndex];
    }

    bool VehicleManager::loadConfig(const std::string& path, VehicleConfig& cfg)
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

            const auto& setters = vehicleConfigSetters();
            const auto setter = setters.find(key);
            if (setter != setters.end())
            {
                setter->second(cfg, val);
            }
        }

        file.close();
        return true;
    }

    const VehicleConfig* VehicleManager::findConfig(const std::string& id) const
    {
        for (const auto& cfg : m_Registry)
        {
            if (cfg.id == id)
                return &cfg;
        }
        return nullptr;
    }

} // namespace bunker
