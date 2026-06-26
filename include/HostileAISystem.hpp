#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Collisions.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace bunker {

// ══════════════════════════════════════════════════════════════════════
// HostileAISystem
// Перенос механик из GMyGameDoNotTouch/GameRuntime.cpp:
//   - 4 архетипа врагов: VerminRush, GhoulRush, HumanTactical, RobotControl
//   - Awareness / Aggro: враги не атакуют мгновенно, сначала замечают цель
//   - Разные радиусы обнаружения, скорости, урон, кулдауны атак
//   - Механические враги получают компонентный урон:
//       sensors / weapon / mobility
//   - Роботы деградируют от повреждений: хуже видят, медленнее ходят,
//     слабее атакуют
//
// ВАЖНО:
// Текущий Enemy в Izometrical_Project пока минимальный. Чтобы не ломать
// существующие сейвы и структуры, расширенное состояние хранится здесь
// параллельно в m_State и синхронизируется по индексу с gs.enemies.
// ══════════════════════════════════════════════════════════════════════

enum class HostileKind {
    VerminRush,      // быстрый рой, малый урон, короткий радиус
    GhoulRush,       // живучий мутант, рывок в ближний бой
    HumanTactical,   // человек: держит дистанцию, флангует, стреляет
    RobotControl     // механический враг: сенсоры/оружие/ходовая
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

    // ── Создать врага с полноценным AI-профилем ──
    // Можно использовать вместо прямого gs.enemies.push_back(...).
    int spawnHostile(GameState& gs, HostileKind kind, const Vector3D& pos) {
        Enemy e;
        e.position = pos;
        e.isAlive = true;

        HostileProfile p = profileFor(kind);
        e.speed = p.speed;
        e.radius = radiusFor(kind);
        e.health = healthFor(kind);

        gs.enemies.push_back(e);
        ensureStateSize(gs);

        const int index = static_cast<int>(gs.enemies.size()) - 1;
        m_State[index] = makeRuntimeState(kind);
        return index;
    }

    // ── Назначить тип уже существующему Enemy ──
    void assignKind(GameState& gs, std::size_t enemyIndex, HostileKind kind) {
        ensureStateSize(gs);
        if (enemyIndex >= gs.enemies.size()) return;

        m_State[enemyIndex] = makeRuntimeState(kind);
        HostileProfile p = profileFor(kind);
        gs.enemies[enemyIndex].speed = p.speed;
        gs.enemies[enemyIndex].radius = radiusFor(kind);
        if (gs.enemies[enemyIndex].health <= 0.0f) {
            gs.enemies[enemyIndex].health = healthFor(kind);
        }
    }

    // ── Главный update всех врагов ──
    void update(GameState& gs, float dt) {
        ensureStateSize(gs);

        for (std::size_t i = 0; i < gs.enemies.size(); ++i) {
            Enemy& e = gs.enemies[i];
            HostileRuntimeState& st = m_State[i];

            if (!e.isAlive) continue;
            if (e.health <= 0.0f) {
                e.isAlive = false;
                continue;
            }

            HostileProfile p = profileFor(st.kind);
            applyMechanicalDegradation(p, st, e);

            if (st.attackTimer > 0.0f) st.attackTimer -= dt;
            if (st.strafeTimer > 0.0f) st.strafeTimer -= dt;
            if (st.strafeTimer <= 0.0f) {
                st.strafeTimer = 1.0f + random01() * 1.5f;
                st.strafeSign = (random01() < 0.5f) ? -1.0f : 1.0f;
            }

            Vector3D target = chooseTarget(gs, e);
            float dx = target.x - e.position.x;
            float dy = target.y - e.position.y;
            float distSq = dx * dx + dy * dy;
            float dist = std::sqrt(std::max(distSq, 0.0001f));

            updateAwareness(st, p, target, dist, dt);

            switch (st.alert) {
                case HostileAlertState::Idle:
                    updateIdle(e, dt);
                    break;

                case HostileAlertState::Suspicious:
                    moveToward(gs, e, st.lastKnownTarget, p.speed * 0.55f, dt);
                    break;

                case HostileAlertState::Aggro:
                    updateAggro(gs, e, st, p, target, dist, dt);
                    break;

                case HostileAlertState::Searching:
                    updateSearching(gs, e, st, p, dt);
                    break;
            }
        }
    }

    // ── Компонентный урон по механическому врагу ──
    // hitLocalX условно задаёт часть корпуса:
    //   < -0.33 = mobility, > 0.33 = weapon, середина = sensors/core.
    void applyDamageToHostile(GameState& gs, std::size_t enemyIndex, float damage, float hitLocalX = 0.0f) {
        ensureStateSize(gs);
        if (enemyIndex >= gs.enemies.size()) return;

        Enemy& e = gs.enemies[enemyIndex];
        HostileRuntimeState& st = m_State[enemyIndex];

        if (!e.isAlive) return;

        if (profileFor(st.kind).isMechanical) {
            if (hitLocalX < -0.33f) {
                st.mech.mobility = std::max(0.0f, st.mech.mobility - damage * 1.15f);
            } else if (hitLocalX > 0.33f) {
                st.mech.weapon = std::max(0.0f, st.mech.weapon - damage * 1.10f);
            } else {
                st.mech.sensors = std::max(0.0f, st.mech.sensors - damage * 0.90f);
            }

            // Робот получает часть урона в общий HP, но главный эффект — деградация узлов.
            e.health -= damage * 0.65f;
        } else {
            e.health -= damage;
        }

        st.awareness = std::min(profileFor(st.kind).aggroThreshold, st.awareness + 45.0f);
        st.alert = HostileAlertState::Aggro;
        st.lastKnownTarget = gs.playerPos;

        if (e.health <= 0.0f) {
            e.isAlive = false;
            gs.score += profileFor(st.kind).isMechanical ? 250 : 150;
        }
    }

    const std::vector<HostileRuntimeState>& debugStates() const { return m_State; }

private:
    void ensureStateSize(const GameState& gs) {
        if (m_State.size() == gs.enemies.size()) return;

        std::size_t oldSize = m_State.size();
        m_State.resize(gs.enemies.size());

        for (std::size_t i = oldSize; i < m_State.size(); ++i) {
            // Безопасная авто-классификация старых Enemy, чтобы текущие карты работали.
            HostileKind kind = HostileKind::VerminRush;
            if (i % 4 == 1) kind = HostileKind::GhoulRush;
            if (i % 4 == 2) kind = HostileKind::HumanTactical;
            if (i % 4 == 3) kind = HostileKind::RobotControl;
            m_State[i] = makeRuntimeState(kind);
        }
    }

    HostileRuntimeState makeRuntimeState(HostileKind kind) const {
        HostileRuntimeState st;
        st.kind = kind;
        st.alert = HostileAlertState::Idle;
        st.awareness = 0.0f;
        st.attackTimer = random01() * 0.4f;
        st.strafeSign = (random01() < 0.5f) ? -1.0f : 1.0f;
        st.strafeTimer = 0.5f + random01();
        return st;
    }

    static HostileProfile profileFor(HostileKind kind) {
        HostileProfile p;
        p.kind = kind;

        switch (kind) {
            case HostileKind::VerminRush:
                p.detectRadius = 5.5f;
                p.loseRadius = 8.0f;
                p.attackRadius = 0.55f;
                p.preferredRange = 0.45f;
                p.speed = 3.15f;
                p.damage = 4.0f;
                p.attackCooldown = 0.55f;
                p.awarenessGain = 75.0f;
                p.awarenessDecay = 26.0f;
                break;

            case HostileKind::GhoulRush:
                p.detectRadius = 6.5f;
                p.loseRadius = 10.0f;
                p.attackRadius = 0.75f;
                p.preferredRange = 0.60f;
                p.speed = 2.35f;
                p.damage = 10.0f;
                p.attackCooldown = 1.05f;
                p.awarenessGain = 58.0f;
                p.awarenessDecay = 16.0f;
                break;

            case HostileKind::HumanTactical:
                p.detectRadius = 8.5f;
                p.loseRadius = 12.0f;
                p.attackRadius = 6.0f;
                p.preferredRange = 4.5f;
                p.speed = 2.15f;
                p.damage = 8.0f;
                p.attackCooldown = 1.15f;
                p.awarenessGain = 48.0f;
                p.awarenessDecay = 14.0f;
                p.usesRangedAttack = true;
                break;

            case HostileKind::RobotControl:
                p.detectRadius = 9.0f;
                p.loseRadius = 13.5f;
                p.attackRadius = 5.5f;
                p.preferredRange = 4.0f;
                p.speed = 1.85f;
                p.damage = 12.0f;
                p.attackCooldown = 1.35f;
                p.awarenessGain = 62.0f;
                p.awarenessDecay = 10.0f;
                p.isMechanical = true;
                p.usesRangedAttack = true;
                break;
        }

        return p;
    }

    static float healthFor(HostileKind kind) {
        switch (kind) {
            case HostileKind::VerminRush:    return 35.0f;
            case HostileKind::GhoulRush:     return 85.0f;
            case HostileKind::HumanTactical: return 65.0f;
            case HostileKind::RobotControl:  return 140.0f;
        }
        return Config::ENEMY_BASE_HP;
    }

    static float radiusFor(HostileKind kind) {
        switch (kind) {
            case HostileKind::VerminRush:    return 0.22f;
            case HostileKind::GhoulRush:     return 0.34f;
            case HostileKind::HumanTactical: return 0.28f;
            case HostileKind::RobotControl:  return 0.42f;
        }
        return Config::ENEMY_RADIUS;
    }

    void applyMechanicalDegradation(HostileProfile& p, const HostileRuntimeState& st, Enemy& e) const {
        if (!p.isMechanical) return;

        float sensorFactor = std::clamp(st.mech.sensors / 100.0f, 0.25f, 1.0f);
        float weaponFactor = std::clamp(st.mech.weapon / 100.0f, 0.15f, 1.0f);
        float moveFactor   = std::clamp(st.mech.mobility / 100.0f, 0.20f, 1.0f);

        p.detectRadius *= sensorFactor;
        p.loseRadius   *= std::max(sensorFactor, 0.45f);
        p.damage       *= weaponFactor;
        p.speed        *= moveFactor;
        e.speed = p.speed;
    }

    void updateAwareness(HostileRuntimeState& st, const HostileProfile& p,
                         const Vector3D& target, float dist, float dt) {
        bool canSense = dist <= p.detectRadius;
        bool tooFar = dist > p.loseRadius;

        if (canSense) {
            st.awareness = std::min(p.aggroThreshold, st.awareness + p.awarenessGain * dt);
            st.lastKnownTarget = target;

            if (st.awareness >= p.aggroThreshold) {
                st.alert = HostileAlertState::Aggro;
                st.searchTimer = 2.5f;
            } else if (st.alert == HostileAlertState::Idle) {
                st.alert = HostileAlertState::Suspicious;
            }
        } else {
            st.awareness = std::max(0.0f, st.awareness - p.awarenessDecay * dt);

            if (st.alert == HostileAlertState::Aggro && tooFar) {
                st.alert = HostileAlertState::Searching;
                st.searchTimer = 3.0f;
            } else if (st.awareness <= 0.0f) {
                st.alert = HostileAlertState::Idle;
            }
        }
    }

    void updateIdle(Enemy& e, float dt) const {
        // Лёгкое шевеление роя, чтобы враги не выглядели мёртвыми.
        float wobbleX = (random01() - 0.5f) * 0.15f * dt;
        float wobbleY = (random01() - 0.5f) * 0.15f * dt;
        e.position.x += wobbleX;
        e.position.y += wobbleY;
    }

    void updateSearching(GameState& gs, Enemy& e, HostileRuntimeState& st,
                         const HostileProfile& p, float dt) {
        st.searchTimer -= dt;
        moveToward(gs, e, st.lastKnownTarget, p.speed * 0.70f, dt);

        if (st.searchTimer <= 0.0f) {
            st.alert = HostileAlertState::Suspicious;
        }
    }

    void updateAggro(GameState& gs, Enemy& e, HostileRuntimeState& st,
                     const HostileProfile& p, const Vector3D& target,
                     float dist, float dt) {
        if (p.usesRangedAttack) {
            updateRanged(gs, e, st, p, target, dist, dt);
        } else {
            updateMelee(gs, e, st, p, target, dist, dt);
        }
    }

    void updateMelee(GameState& gs, Enemy& e, HostileRuntimeState& st,
                     const HostileProfile& p, const Vector3D& target,
                     float dist, float dt) {
        if (dist > p.attackRadius) {
            float rush = (st.kind == HostileKind::GhoulRush && dist < 2.3f) ? 1.35f : 1.0f;
            moveToward(gs, e, target, p.speed * rush, dt);
            return;
        }

        performAttack(gs, st, p);
    }

    void updateRanged(GameState& gs, Enemy& e, HostileRuntimeState& st,
                      const HostileProfile& p, const Vector3D& target,
                      float dist, float dt) {
        if (dist > p.attackRadius) {
            moveToward(gs, e, target, p.speed, dt);
        } else if (dist < p.preferredRange * 0.65f) {
            moveAway(gs, e, target, p.speed * 0.85f, dt);
        } else {
            strafeAround(gs, e, target, p.speed * 0.55f, st.strafeSign, dt);
        }

        if (dist <= p.attackRadius) {
            performAttack(gs, st, p);
        }
    }

    void performAttack(GameState& gs, HostileRuntimeState& st, const HostileProfile& p) {
        if (st.attackTimer > 0.0f) return;

        // Если игрок внутри Титана — бьём Титан. Иначе игрока.
        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted) {
            gs.titan.health = std::max(0.0f, gs.titan.health - p.damage);
            if (p.isMechanical) {
                gs.titan.systems.sensorLink = std::max(0.0f, gs.titan.systems.sensorLink - p.damage * 0.25f);
            }
        } else {
            gs.playerHealth = std::max(0.0f, gs.playerHealth - p.damage);
        }

        st.attackTimer = p.attackCooldown;
    }

    Vector3D chooseTarget(const GameState& gs, const Enemy& e) const {
        // Титан-пилотируемый — более заметная цель.
        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted) {
            return gs.titan.position;
        }

        // Если автономный Титан ближе игрока, часть врагов может отвлечься на него.
        float pdx = gs.playerPos.x - e.position.x;
        float pdy = gs.playerPos.y - e.position.y;
        float tdx = gs.titan.position.x - e.position.x;
        float tdy = gs.titan.position.y - e.position.y;
        float playerDistSq = pdx * pdx + pdy * pdy;
        float titanDistSq = tdx * tdx + tdy * tdy;

        if (gs.titan.health > 0.0f && titanDistSq + 4.0f < playerDistSq) {
            return gs.titan.position;
        }

        return gs.playerPos;
    }

    void moveToward(GameState& gs, Enemy& e, const Vector3D& target, float speed, float dt) const {
        float dx = target.x - e.position.x;
        float dy = target.y - e.position.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) return;
        tryMove(gs, e, dx / len, dy / len, speed, dt);
    }

    void moveAway(GameState& gs, Enemy& e, const Vector3D& target, float speed, float dt) const {
        float dx = e.position.x - target.x;
        float dy = e.position.y - target.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) return;
        tryMove(gs, e, dx / len, dy / len, speed, dt);
    }

    void strafeAround(GameState& gs, Enemy& e, const Vector3D& target,
                      float speed, float sign, float dt) const {
        float dx = target.x - e.position.x;
        float dy = target.y - e.position.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) return;

        float nx = dx / len;
        float ny = dy / len;

        // Перпендикуляр для фланга.
        tryMove(gs, e, -ny * sign, nx * sign, speed, dt);
    }

    void tryMove(GameState& gs, Enemy& e, float dirX, float dirY, float speed, float dt) const {
        float nx = e.position.x + dirX * speed * dt;
        float ny = e.position.y + dirY * speed * dt;

        if (!Collisions::checkWorldCollision(gs, nx, ny, e.radius)) {
            e.position.x = nx;
            e.position.y = ny;
            return;
        }

        // Sliding по X/Y, чтобы враги не залипали в углах.
        if (!Collisions::checkWorldCollision(gs, nx, e.position.y, e.radius)) {
            e.position.x = nx;
        }
        if (!Collisions::checkWorldCollision(gs, e.position.x, ny, e.radius)) {
            e.position.y = ny;
        }
    }

    static float random01() {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }
};

}  // namespace bunker
