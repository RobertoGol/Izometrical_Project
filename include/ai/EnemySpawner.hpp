#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "entities/Collisions.hpp"
#include "core/IsoMath.hpp"
#include "core/Constants.hpp"
#include <string>
#include <vector>

namespace bunker {

enum class EnemyClassType {
    VerminSwarm,
    TargetDummy,
    PunchingBag,
    StaticBase
};

struct EnemyConfig {
    std::string    id;
    std::string    displayName = "UNKNOWN ENTITY";
    EnemyClassType classType   = EnemyClassType::VerminSwarm;
    float maxHealth            = 35.0f;
    float baseSpeed            = 2.4f;
    float physicalRadius       = 0.35f;
    float erosionDamage        = 0.0f;
    int   rewardScore          = 150;
    std::string factionID      = "Swarm";
    std::string texturePath;
    std::string soundDeathPath;
    bool  loaded               = false;
};

struct EnemyExtended {
    const EnemyConfig* config = nullptr;
    Vector3D position;
    float    health   = 35.0f;
    float    speed    = 2.4f;
    float    radius   = 0.35f;
    bool     isAlive  = true;
};

class EnemySpawner {
private:
    std::vector<EnemyConfig> m_Registry;
    float m_SpawnTimer = 0.0f;

public:
    EnemySpawner() = default;

    void scanAndLoadConfigs(const std::string& enemiesDir = "assets/enemies");
    bool spawnByConfigId(GameState& gs, const std::string& configId, const Vector3D& pos);
    void updateWaveSpawning(GameState& gs, float dt);
    void updateEnemyAI(GameState& gs, float dt);
    void spawnTrainingZone(GameState& gs);

    const std::vector<EnemyConfig>& getRegistry() const { return m_Registry; }

private:
    bool loadConfig(const std::string& path, EnemyConfig& cfg);
    const EnemyConfig* findConfig(const std::string& id) const;
};

}  // namespace bunker
