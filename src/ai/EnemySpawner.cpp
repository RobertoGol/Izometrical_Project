#include "ai/EnemySpawner.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace bunker {

void EnemySpawner::scanAndLoadConfigs(const std::string& enemiesDir) {
    m_Registry.clear();

    if (!std::filesystem::exists(enemiesDir)) {
        std::cout << "[ENEMIES] Папка " << enemiesDir << " не найдена, пропускаем." << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(enemiesDir)) {
        if (!entry.is_directory()) continue;

        std::string folderName = entry.path().filename().string();
        std::string cfgPath = entry.path().string() + "/enemy.cfg";

        if (!std::filesystem::exists(cfgPath)) continue;

        EnemyConfig cfg;
        cfg.id = folderName;

        if (loadConfig(cfgPath, cfg)) {
            if (!cfg.texturePath.empty())
                cfg.texturePath = entry.path().string() + "/" + cfg.texturePath;
            if (!cfg.soundDeathPath.empty())
                cfg.soundDeathPath = entry.path().string() + "/" + cfg.soundDeathPath;

            cfg.loaded = true;
            m_Registry.push_back(cfg);
            std::cout << "[ENEMIES] Зарегистрирован: " << cfg.displayName
                      << " [" << cfg.id << "] hp=" << cfg.maxHealth
                      << " spd=" << cfg.baseSpeed << std::endl;
        }
    }

    std::cout << "[ENEMIES] Всего типов врагов: " << m_Registry.size() << std::endl;
}

bool EnemySpawner::spawnByConfigId(GameState& gs, const std::string& configId, const Vector3D& pos) {
    const EnemyConfig* cfg = findConfig(configId);
    if (!cfg) {
        Enemy e;
        e.position = pos;
        e.health   = Config::ENEMY_BASE_HP;
        e.speed    = Config::ENEMY_BASE_SPEED;
        e.radius   = Config::ENEMY_RADIUS;
        e.isAlive  = true;
        gs.enemies.push_back(e);
        return false;
    }

    Enemy e;
    e.position = pos;
    e.health   = cfg->maxHealth;
    e.speed    = cfg->baseSpeed;
    e.radius   = cfg->physicalRadius;
    e.isAlive  = true;
    gs.enemies.push_back(e);
    return true;
}

void EnemySpawner::updateWaveSpawning(GameState& gs, float dt) {
    m_SpawnTimer += dt;

    if (m_SpawnTimer >= Config::ENEMY_SPAWN_INTERVAL) {
        m_SpawnTimer = 0.0f;

        if (static_cast<int>(gs.enemies.size()) < Config::ENEMY_MAX_COUNT) {
            Vector3D spawnPoints[4] = {
                {1.0f, 1.0f, 0.0f},
                {18.0f, 1.0f, 0.0f},
                {1.0f, 18.0f, 0.0f},
                {18.0f, 18.0f, 0.0f}
            };

            for (int i = 0; i < 4; ++i) {
                Enemy e;
                e.position = spawnPoints[i];
                e.health   = Config::ENEMY_BASE_HP;
                e.speed    = 2.2f + static_cast<float>(rand() % 100) / 500.0f;
                e.isAlive  = true;
                e.radius   = 0.32f;
                gs.enemies.push_back(e);
            }
        }
    }
}

void EnemySpawner::updateEnemyAI(GameState& gs, float dt) {
    for (auto& e : gs.enemies) {
        if (!e.isAlive) continue;

        if (e.speed < 0.01f) continue;

        float dx = gs.playerPos.x - e.position.x;
        float dy = gs.playerPos.y - e.position.y;
        float distToPlayerSq = dx * dx + dy * dy;

        float targetX = gs.towerPosition.x;
        float targetY = gs.towerPosition.y;

        if (distToPlayerSq < Config::ENEMY_AGGRO_RANGE_SQ) {
            targetX = gs.playerPos.x;
            targetY = gs.playerPos.y;
        }

        float tdx = targetX - e.position.x;
        float tdy = targetY - e.position.y;
        float len = std::sqrt(tdx * tdx + tdy * tdy);

        if (len > 0.05f) {
            e.position.x += (tdx / len) * e.speed * dt;
            e.position.y += (tdy / len) * e.speed * dt;
        }
    }

    gs.enemies.erase(
        std::remove_if(gs.enemies.begin(), gs.enemies.end(),
            [](const Enemy& e) { return !e.isAlive; }),
        gs.enemies.end()
    );
}

void EnemySpawner::spawnTrainingZone(GameState& gs) {
    spawnByConfigId(gs, "target_dummy",  {14.0f, 14.0f, 0.0f});
    spawnByConfigId(gs, "punching_bag",  {14.0f, 15.0f, 0.0f});
    spawnByConfigId(gs, "static_base",   {15.0f, 14.0f, 0.0f});
}

bool EnemySpawner::loadConfig(const std::string& path, EnemyConfig& cfg) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '/') continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        if      (key == "name")           cfg.displayName    = val;
        else if (key == "health")         cfg.maxHealth      = std::stof(val);
        else if (key == "speed")          cfg.baseSpeed      = std::stof(val);
        else if (key == "radius")         cfg.physicalRadius = std::stof(val);
        else if (key == "erosion_damage") cfg.erosionDamage  = std::stof(val);
        else if (key == "reward_score")   cfg.rewardScore    = std::stoi(val);
        else if (key == "faction")        cfg.factionID      = val;
        else if (key == "texture")        cfg.texturePath    = val;
        else if (key == "sound_death")    cfg.soundDeathPath = val;
        else if (key == "type") {
            if      (val == "VerminSwarm")  cfg.classType = EnemyClassType::VerminSwarm;
            else if (val == "TargetDummy")  cfg.classType = EnemyClassType::TargetDummy;
            else if (val == "PunchingBag")  cfg.classType = EnemyClassType::PunchingBag;
            else if (val == "StaticBase")   cfg.classType = EnemyClassType::StaticBase;
        }
    }
    file.close();
    return true;
}

const EnemyConfig* EnemySpawner::findConfig(const std::string& id) const {
    for (const auto& cfg : m_Registry) {
        if (cfg.id == id) return &cfg;
    }
    return nullptr;
}

}  // namespace bunker
