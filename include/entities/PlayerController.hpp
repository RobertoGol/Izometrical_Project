#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "InputManager.hpp"
#include "IsoMath.hpp"
#include "Constants.hpp"
#include <cmath>
#include <algorithm>

namespace bunker
{

    class PlayerController
    {
    private:
        // ── Инерция / плавное движение ──
        Vector3D m_Velocity = {0.0f, 0.0f, 0.0f};
        float m_Acceleration = 12.0f;
        float m_Deceleration = 8.0f;

        // ── Стамина ──
        float m_CurrentStamina = Config::STAMINA_MAX;
        float m_MaxStamina = Config::STAMINA_MAX;
        float m_StaminaDrain = Config::STAMINA_DRAIN;
        float m_StaminaRegen = Config::STAMINA_REGEN;

        // ── Скорости ──
        float m_WalkSpeed = Config::PLAYER_WALK_SPEED;
        float m_SprintSpeed = Config::PLAYER_SPRINT_SPEED;

        // ── Нырок (Dive — Helldivers 2) ──
        bool m_IsDiving = false;
        float m_DiveTimer = 0.0f;
        Vector3D m_DiveDirection = {0.0f, 0.0f, 0.0f};

        // ── Угол взгляда ──
        float m_FacingAngle = 0.0f;

    public:
        PlayerController() = default;

        // Вызывается каждый кадр. Обновляет позицию, скорость, стамину, нырок.
        void update(GameState &gs, const InputSnapshot &input, float dt);

        // Геттеры
        float getFacingAngle() const { return m_FacingAngle; }
        float getStamina() const { return m_CurrentStamina; }
        float getMaxStamina() const { return m_MaxStamina; }
        float getStaminaPercent() const { return m_CurrentStamina / m_MaxStamina; }
        bool isDiving() const { return m_IsDiving; }
        const Vector3D &getVelocity() const { return m_Velocity; }
    };

} // namespace bunker
