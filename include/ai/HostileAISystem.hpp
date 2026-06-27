#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "entities/Collisions.hpp"
#include "core/Constants.hpp"
#include <vector>
#include <cstddef>

namespace bunker {

enum class HostileKind {
    VerminRush,
    GhoulRush,
    HumanTactical,
    RobotControl
};

enum class HostileAlertState {
    Idle,
    Suspicious,
    Aggro,
    Searching
};

struct HostileProfile {
    HostileKind kind = HostileKind::VerminRush;
    float detectRadius      = 6.0f;
    float loseRadius        = 9.0f;
    float attackRadius      = 0.65f;
    float preferredRange    = 0.65f;
    float speed             = 2.0f;
    float damage            = 6.0f;
    float attackCooldown    = 0.9f;
    float awarenessGain     = 55.0f;
    float awarenessDecay    = 18.0f;
    float aggroThreshold    = 100.0f;
    bool  isMechanical      = false;
    bool  usesRangedAttack  = false;
};

struct MechanicalDamageState {
    float sensors  = 100.0f;
    float weapon   = 100.0f;
    float mobility = 100.0f;
    bool sensorsBroken()  const { return sensors  <= 0.0f; }
    bool weaponBroken()   const { return weapon   <= 0.0f; }
    bool mobilityBroken() const { return mobility <= 0.0f; }
};

struct HostileRuntimeState {
    HostileKind       kind       = HostileKind::VerminRush;
    HostileAlertState alert      = HostileAlertState::Idle;
    MechanicalDamageState mech;
    float awareness     = 0.0f;
    float attackTimer   = 0.0f;
    float searchTimer   = 0.0f;
    float strafeSign    = 1.0f;
    float strafeTimer   = 0.0f;
    Vector3D lastKnownTarget = {0.0f, 0.0f, 0.0f};
};

class HostileAISystem {
private:
    std::vector<HostileRuntimeState> m_State;

public:
    HostileAISystem() = default;

    int spawnHostile(GameState& gs, HostileKind kind, const Vector3D& pos);
    void assignKind(GameState& gs, std::size_t enemyIndex, HostileKind kind);
    void update(GameState& gs, float dt);
    void applyDamageToHostile(GameState& gs, std::size_t enemyIndex, float damage, float hitLocalX = 0.0f);
    const std::vector<HostileRuntimeState>& debugStates() const { return m_State; }

    static HostileProfile profileFor(HostileKind kind);
    static float radiusFor(HostileKind kind);
    static float healthFor(HostileKind kind);

private:
    void ensureStateSize(const GameState& gs);
    static HostileRuntimeState makeRuntimeState(HostileKind kind);
    void applyMechanicalDegradation(HostileProfile& p, const HostileRuntimeState& st, Enemy& e) const;
    void updateAwareness(HostileRuntimeState& st, const HostileProfile& p, float distSq, float visibilityMultiplier, float dt) const;
    void updateIdle(Enemy& e, float dt) const;
    void updateSearching(GameState& gs, Enemy& e, HostileRuntimeState& st, float speed, float dt) const;
    void updateAggro(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p, float dt);
    void updateMelee(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p, const Vector3D& target, float distSq, float dt);
    void updateRanged(GameState& gs, Enemy& e, HostileRuntimeState& st, const HostileProfile& p, const Vector3D& target, float distSq, float dt);
    void performAttack(GameState& gs, HostileRuntimeState& st, const HostileProfile& p);
    Vector3D findPreferredTarget(const GameState& gs, const Enemy& e) const;
    void moveToward(GameState& gs, Enemy& e, const Vector3D& target, float speed, float dt) const;
    void moveAway(GameState& gs, Enemy& e, const Vector3D& target, float speed, float dt) const;
    void strafeAround(GameState& gs, Enemy& e, const Vector3D& target, float speed, float& strafeSign, float& strafeTimer, float dt) const;
    void tryMove(GameState& gs, Enemy& e, float dirX, float dirY, float speed, float dt) const;
    static float distSq(const Vector3D& a, const Vector3D& b);
};

}  // namespace bunker
