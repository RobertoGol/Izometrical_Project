#include "PlayerController.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/PhysicsWorld.hpp"
#include <cmath>
#include <algorithm>

namespace bunker
{

    // Запоминаем последнее направление камеры.
    // Если мышь встанет ровно на игрока — используем последние оси.
    // Стартовое значение — стандартные изометрические оси.
    static Vector3D s_LastCamForward = {0.707f, -0.707f, 0.0f};
    static Vector3D s_LastCamRight = {0.707f, 0.707f, 0.0f};

    void PlayerController::update(GameState &gs, const InputSnapshot &input, float dt)
    {
        float playerRadius = (gs.playerMode == UnitMode::Titan) ? Config::TITAN_RADIUS : Config::PLAYER_RADIUS;

        gs.isAiming = input.isAiming;
        if (gs.fireCooldown > 0.0f)
        {
            gs.fireCooldown -= dt;
        }

        updateCameraAxes(gs);

        Vector3D targetDir = buildMovementDirection(input);
        bool isMoving = targetDir.lengthSq() > 0.0001f;
        bool activeSprint = updateStamina(gs, input, isMoving, dt);

        if (updateDive(gs, input, targetDir, isMoving, playerRadius, dt))
        {
            return;
        }

        applyMovement(gs, targetDir, isMoving, activeSprint, playerRadius, dt);
        updateFacingAngle(gs);
    }

    void PlayerController::updateCameraAxes(const GameState& gs)
    {
        float camDx = gs.mouseWorldPos.x - gs.playerPos.x;
        float camDy = gs.mouseWorldPos.y - gs.playerPos.y;
        float camLen = std::sqrt(camDx * camDx + camDy * camDy);

        if (camLen > 0.01f)
        {
            s_LastCamForward.x = camDx / camLen;
            s_LastCamForward.y = camDy / camLen;
            s_LastCamForward.z = 0.0f;

            s_LastCamRight.x = s_LastCamForward.y;
            s_LastCamRight.y = -s_LastCamForward.x;
            s_LastCamRight.z = 0.0f;
        }
    }

    Vector3D PlayerController::buildMovementDirection(const InputSnapshot& input) const
    {
        Vector3D targetDir = {0.0f, 0.0f, 0.0f};
        targetDir.x = s_LastCamForward.x * input.moveForward + s_LastCamRight.x * input.moveStrafe;
        targetDir.y = s_LastCamForward.y * input.moveForward + s_LastCamRight.y * input.moveStrafe;

        float dirLen = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
        if (dirLen > 0.01f)
        {
            targetDir.x /= dirLen;
            targetDir.y /= dirLen;
        }

        return targetDir;
    }

    bool PlayerController::updateStamina(const GameState& gs, const InputSnapshot& input, bool isMoving, float dt)
    {
        bool canSprint = (m_CurrentStamina > 0.0f) && isMoving && (gs.playerMode == UnitMode::Scout);
        bool activeSprint = input.isSprinting && canSprint;

        if (activeSprint)
        {
            m_CurrentStamina = std::max(0.0f, m_CurrentStamina - m_StaminaDrain * dt);
        }
        else
        {
            m_CurrentStamina = std::min(m_MaxStamina, m_CurrentStamina + m_StaminaRegen * dt);
        }

        return activeSprint;
    }

    bool PlayerController::updateDive(GameState& gs, const InputSnapshot& input, const Vector3D& targetDir,
                                      bool isMoving, float playerRadius, float dt)
    {
        if (gs.playerMode != UnitMode::Scout)
        {
            return false;
        }

        if (input.isDiving && !m_IsDiving && isMoving)
        {
            m_IsDiving = true;
            m_DiveTimer = Config::PLAYER_DIVE_TIME;
            m_DiveDirection = targetDir;
            m_DiveDirection.z = 0.0f;
        }

        if (!m_IsDiving)
        {
            return false;
        }

        m_DiveTimer -= dt;
        if (m_DiveTimer <= 0.0f)
        {
            m_IsDiving = false;
            gs.playerPos.z = 0.0f;
            return true;
        }

        float nextX = gs.playerPos.x + m_DiveDirection.x * Config::PLAYER_DIVE_SPEED * dt;
        float nextY = gs.playerPos.y + m_DiveDirection.y * Config::PLAYER_DIVE_SPEED * dt;

        PhysicsWorld physicsWorld(gs);
        CollisionProbe probe{};
        probe.position = gs.playerPos;
        probe.radius = playerRadius;
        probe.height = 1.8f;

        const CollisionMoveResult move =
            CollisionSystem::sweepAndSlide(physicsWorld,
                                           probe,
                                           {nextX - gs.playerPos.x, nextY - gs.playerPos.y, 0.0f});
        gs.playerPos = move.position;

        float progress = 1.0f - (m_DiveTimer / Config::PLAYER_DIVE_TIME);
        gs.playerPos.z = std::sin(progress * 3.14159265f) * 0.5f;
        return true;
    }

    void PlayerController::applyMovement(GameState& gs, const Vector3D& targetDir, bool isMoving, bool activeSprint,
                                         float playerRadius, float dt)
    {
        Vector3D targetVelocity = {0.0f, 0.0f, 0.0f};

        if (isMoving)
        {
            float currentMoveSpeed = activeSprint ? m_SprintSpeed : m_WalkSpeed;

            if (gs.playerMode == UnitMode::Scout && gs.isAiming)
            {
                currentMoveSpeed *= 0.63f;
            }
            if (gs.playerMode == UnitMode::Titan && gs.titan.systems.tracksCondition < 40.0f)
            {
                currentMoveSpeed *= 0.3f;
            }

            targetVelocity.x = targetDir.x * currentMoveSpeed;
            targetVelocity.y = targetDir.y * currentMoveSpeed;
        }

        float currentLerp = isMoving ? m_Acceleration : m_Deceleration;
        m_Velocity.x += (targetVelocity.x - m_Velocity.x) * currentLerp * dt;
        m_Velocity.y += (targetVelocity.y - m_Velocity.y) * currentLerp * dt;

        float nextX = gs.playerPos.x + m_Velocity.x * dt;
        float nextY = gs.playerPos.y + m_Velocity.y * dt;

        PhysicsWorld physicsWorld(gs);
        CollisionProbe probe{};
        probe.position = gs.playerPos;
        probe.radius = playerRadius;
        probe.height = 1.8f;

        const CollisionMoveResult move =
            CollisionSystem::sweepAndSlide(physicsWorld,
                                           probe,
                                           {nextX - gs.playerPos.x, nextY - gs.playerPos.y, 0.0f});
        gs.playerPos = move.position;
    }

    void PlayerController::updateFacingAngle(const GameState& gs)
    {
        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;

        if (std::abs(dx) > 0.05f || std::abs(dy) > 0.05f)
        {
            m_FacingAngle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
            if (m_FacingAngle < 0.0f)
            {
                m_FacingAngle += 360.0f;
            }
        }
    }
} // namespace bunker
