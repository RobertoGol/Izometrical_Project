#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Collisions.hpp"
#include "Constants.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace bunker {

// Типы аналогового вооружения БТ-7274
enum class AncientLoadout {
    XO16_SolidKinetic,       // Автопушка XO-16
    Scorch_ThermiteMortar,   // Миномёт Скорча
    Ion_SplitLaser_Vacuum    // Ион-лазер
};

// Баллистический профиль кулачковых валов
struct BallisticCamProfile {
    float hydraulic_pressure_bar = 350.0f;
    float recoil_decay_ms        = 120.0f;
    float mechanical_lead_factor = 1.02f;
};

class TitanAI {
private:
    // ── Физика шасси ──
    const float m_TitanAcceleration = 4.0f;
    const float m_TitanDeceleration = 6.0f;
    const float m_ExclusionRadiusSq = 36.0f;  // Anti-Blocking сфера 6м

    Vector3D m_Velocity = {0.0f, 0.0f, 0.0f};

    // ── Вооружение ──
    AncientLoadout     m_ActiveLoadout = AncientLoadout::XO16_SolidKinetic;
    BallisticCamProfile m_CurrentCamProfile;

    // ── Вортекс-щит ──
    bool  m_VortexActive       = false;
    float m_VortexEnergy       = 100.0f;
    int   m_CaughtBulletsCount = 0;

    // ── Ядро (Core Overdrive) ──
    float m_CoreChargePercent   = 0.0f;
    float m_BoilerPressure      = 350.0f;
    float m_CoolantTemperature  = 180.0f;
    bool  m_CoreOverdriveActive = false;

    // ── Механическая смена оружия ──
    float m_ReMapTimer   = 0.0f;
    bool  m_IsReMapping  = false;

public:
    TitanAI() {
        m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
    }

    // ═══════════════════════════════════════════════
    // Главный тик ИИ Титана каждый кадр
    // ═══════════════════════════════════════════════
    void update(GameState& gs, const InputSnapshot& input, float dt) {
        // Накопление заряда Core
        if (!m_CoreOverdriveActive) {
            m_CoreChargePercent = std::min(100.0f, m_CoreChargePercent + 4.5f * dt);
        } else {
            m_CoreChargePercent = std::max(0.0f, m_CoreChargePercent - 18.0f * dt);
            if (m_CoreChargePercent <= 0.0f)
                m_CoreOverdriveActive = false;
        }

        // Таймер кулдауна стрельбы Титана
        if (gs.titan.fireCooldown > 0.0f)
            gs.titan.fireCooldown -= dt;

        // Механическая смена оружия
        if (m_IsReMapping) {
            m_ReMapTimer -= dt;
            if (m_ReMapTimer <= 0.0f)
                m_IsReMapping = false;
            return;
        }

        // Переключение оружия по клавишам 1-2-3 (только в Титане)
        if (gs.playerMode == UnitMode::Titan) {
            if (input.selectClass[0]) triggerReMap(gs, AncientLoadout::XO16_SolidKinetic);
            if (input.selectClass[1]) triggerReMap(gs, AncientLoadout::Scorch_ThermiteMortar);
            if (input.selectClass[2]) triggerReMap(gs, AncientLoadout::Ion_SplitLaser_Vacuum);
        }

        // Вортекс-щит (Q в режиме Титана)
        if (gs.playerMode == UnitMode::Titan) {
            bool holdingQ = input.activateTactical;
            // В SFML activateTactical — это одиночное нажатие Q.
            // Для удержания используем проверку зажатой клавиши:
            holdingQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
            updateVortexShield(gs, holdingQ, dt);
        }

        // ── Автономный режим (Пилот снаружи) ──
        if (!gs.titan.isPiloted) {
            updateAutonomousMode(gs, dt);
            return;
        }

        // ── Симбиотический режим (Пилот внутри) ──
        updatePilotedMode(gs, input, dt);
    }

    // ═══════════════════════════════════════════════
    // Геттеры
    // ═══════════════════════════════════════════════
    bool  isVortexActive()  const { return m_VortexActive; }
    float getVortexEnergy() const { return m_VortexEnergy; }
    bool  isCoreActive()    const { return m_CoreOverdriveActive; }
    float getCoreCharge()   const { return m_CoreChargePercent; }
    AncientLoadout getLoadout() const { return m_ActiveLoadout; }

    // ═══════════════════════════════════════════════
    // Активация Core Overdrive
    // ═══════════════════════════════════════════════
    void executeCoreOverdrive() {
        if (m_CoreChargePercent < 100.0f) return;
        m_CoreOverdriveActive = true;
        m_BoilerPressure = 1200.0f;
        m_CoolantTemperature = 980.0f;
    }

private:
    // ── Смена оружия с задержкой 0.4с ──
    void triggerReMap(GameState& gs, AncientLoadout newWeapon) {
        if (m_VortexActive || m_IsReMapping) return;

        m_ActiveLoadout = newWeapon;
        m_IsReMapping = true;
        m_ReMapTimer = 0.4f;

        switch (newWeapon) {
        case AncientLoadout::XO16_SolidKinetic:
            m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
            gs.titan.currentWeapon = TankWeaponMode::AutoCannon;
            break;
        case AncientLoadout::Scorch_ThermiteMortar:
            m_CurrentCamProfile = {600.0f, 450.0f, 1.85f};
            gs.titan.currentWeapon = TankWeaponMode::Cannon;
            break;
        case AncientLoadout::Ion_SplitLaser_Vacuum:
            m_CurrentCamProfile = {100.0f, 10.0f, 1.0f};
            gs.titan.currentWeapon = TankWeaponMode::AutoCannon;
            break;
        }
    }

    // ── Вортекс-щит: ловит пули, выплёвывает обратно ──
    void updateVortexShield(GameState& gs, bool isHoldingQ, float dt) {
        if (m_ActiveLoadout == AncientLoadout::Scorch_ThermiteMortar) {
            m_VortexActive = false;
            return;
        }

        if (isHoldingQ && m_VortexEnergy > 5.0f) {
            m_VortexActive = true;
            m_VortexEnergy = std::max(0.0f, m_VortexEnergy - 30.0f * dt);

            // Захват вражеских пуль в радиусе 2 клеток
            float shieldRadiusSq = 4.0f;
            for (auto& b : gs.bullets) {
                if (!b.isAlive) continue;
                float bdx = b.current.x - gs.titan.position.x;
                float bdy = b.current.y - gs.titan.position.y;
                if ((bdx * bdx + bdy * bdy) <= shieldRadiusSq) {
                    b.isAlive = false;
                    m_CaughtBulletsCount++;
                }
            }
        } else {
            // Выброс пойманных пуль обратно
            if (m_VortexActive && m_CaughtBulletsCount > 0) {
                float mdx = gs.mouseWorldPos.x - gs.titan.position.x;
                float mdy = gs.mouseWorldPos.y - gs.titan.position.y;
                float mLenSq = mdx * mdx + mdy * mdy;

                if (mLenSq > 0.01f) {
                    float mLen = std::sqrt(mLenSq);
                    Vector3D returnDir = { mdx / mLen, mdy / mLen, 0.0f };

                    for (int i = 0; i < m_CaughtBulletsCount; ++i) {
                        Bullet rb;
                        rb.start   = gs.titan.position;
                        rb.current = gs.titan.position;
                        rb.type    = BulletType::Standard;
                        rb.speed   = 28.0f;
                        float spread = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.25f;
                        rb.direction = { returnDir.x + spread, returnDir.y + spread, 0.0f };
                        gs.bullets.push_back(rb);
                    }
                }
                m_CaughtBulletsCount = 0;
            }

            m_VortexActive = false;
            m_VortexEnergy = std::min(100.0f, m_VortexEnergy + 15.0f * dt);
        }
    }

    // ── Автономный режим: Anti-Blocking + Combat Anchor + автострельба ──
    void updateAutonomousMode(GameState& gs, float dt) {
        float tdx = gs.playerPos.x - gs.titan.position.x;
        float tdy = gs.playerPos.y - gs.titan.position.y;
        float distSq = tdx * tdx + tdy * tdy;

        // Anti-Blocking: уступает дорогу Пилоту
        if (distSq < m_ExclusionRadiusSq) {
            float dist = std::sqrt(distSq);
            if (dist > 0.001f) {
                Vector3D push = { tdx / dist, tdy / dist, 0.0f };
                gs.titan.position.x -= push.x * 12.0f * dt;
                gs.titan.position.y -= push.y * 12.0f * dt;
            }
        } else {
            // Движение к Combat Anchor Point
            Vector3D anchor = calculateCombatAnchor(gs);
            float adx = anchor.x - gs.titan.position.x;
            float ady = anchor.y - gs.titan.position.y;
            float aLenSq = adx * adx + ady * ady;

            if (aLenSq > 1.0f) {
                float aLen = std::sqrt(aLenSq);
                float nextX = gs.titan.position.x + (adx / aLen) * gs.titan.speed * dt;
                float nextY = gs.titan.position.y + (ady / aLen) * gs.titan.speed * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        }
    }

    // ── Симбиотический режим: Пилот управляет БТ ──
    void updatePilotedMode(GameState& gs, const InputSnapshot& input, float dt) {
        Vector3D moveInput = {0.0f, 0.0f, 0.0f};

        if (input.moveForward > 0.0f) { moveInput.x += 1.0f; moveInput.y -= 1.0f; }
        if (input.moveForward < 0.0f) { moveInput.x -= 1.0f; moveInput.y += 1.0f; }
        if (input.moveStrafe  < 0.0f) { moveInput.x -= 1.0f; moveInput.y -= 1.0f; }
        if (input.moveStrafe  > 0.0f) { moveInput.x += 1.0f; moveInput.y += 1.0f; }

        float inputLenSq = moveInput.x * moveInput.x + moveInput.y * moveInput.y;
        Vector3D targetVel = {0.0f, 0.0f, 0.0f};

        if (inputLenSq > 0.001f) {
            float len = std::sqrt(inputLenSq);
            float activeSpeed = m_VortexActive ? (gs.titan.speed * 0.4f) : gs.titan.speed;
            targetVel.x = (moveInput.x / len) * activeSpeed;
            targetVel.y = (moveInput.y / len) * activeSpeed;
        }

        float lerpAcc = (inputLenSq > 0.001f) ? m_TitanAcceleration : m_TitanDeceleration;
        m_Velocity.x += (targetVel.x - m_Velocity.x) * lerpAcc * dt;
        m_Velocity.y += (targetVel.y - m_Velocity.y) * lerpAcc * dt;

        float nextX = gs.titan.position.x + m_Velocity.x * dt;
        float nextY = gs.titan.position.y + m_Velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
            gs.titan.position.x = nextX;
        if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
            gs.titan.position.y = nextY;

        // Привязываем позицию игрока к кабине
        gs.playerPos = gs.titan.position;
    }

    // ── Combat Anchor Point: 40% между Пилотом и центром масс Роя ──
    Vector3D calculateCombatAnchor(const GameState& gs) {
        if (gs.enemies.empty()) return gs.playerPos;

        Vector3D center = {0.0f, 0.0f, 0.0f};
        int count = 0;

        for (const auto& e : gs.enemies) {
            if (!e.isAlive) continue;
            center.x += e.position.x;
            center.y += e.position.y;
            count++;
        }

        if (count == 0) return gs.playerPos;

        center.x /= count;
        center.y /= count;

        return {
            gs.playerPos.x + (center.x - gs.playerPos.x) * 0.40f,
            gs.playerPos.y + (center.y - gs.playerPos.y) * 0.40f,
            0.0f
        };
    }
};

}  // namespace bunker
