#include "PlayerController.hpp"
#include "Collisions.hpp"
#include <cmath>
#include <algorithm>

namespace bunker {

// Статические оси камеры — запоминаем куда смотрела камера в прошлый раз
// Даже если мышь встанет ровно на игрока — движение не пропадёт
static Vector3D s_LastCamForward = {0.707f, -0.707f, 0.0f};
static Vector3D s_LastCamRight   = {0.707f,  0.707f, 0.0f};

void PlayerController::update(GameState& gs, const InputSnapshot& input, float dt) {
    // ── 1. Радиус хитбокса ──
    float playerRadius = (gs.playerMode == UnitMode::Titan)
                         ? Config::TITAN_RADIUS
                         : Config::PLAYER_RADIUS;

    // ── 2. Прицеливание ──
    gs.isAiming = input.isAiming;

    // ── 3. Кулдаун стрельбы ──
    if (gs.fireCooldown > 0.0f)
        gs.fireCooldown -= dt;

    // ══════════════════════════════════════════════════════════
    // 4. Направление движения ОТНОСИТЕЛЬНО КАМЕРЫ
    //
    // Если мышь достаточно далеко от игрока — обновляем оси.
    // Если мышь на игроке (camLen ≈ 0) — используем ПОСЛЕДНИЕ
    // запомненные оси. Т.е. куда камера смотрела — туда и
    // продолжает смотреть.
    // ══════════════════════════════════════════════════════════

    float camDx = gs.mouseWorldPos.x - gs.playerPos.x;
    float camDy = gs.mouseWorldPos.y - gs.playerPos.y;
    float camLen = std::sqrt(camDx * camDx + camDy * camDy);

    if (camLen > 0.01f) {
        // Обновляем оси камеры
        s_LastCamForward.x = camDx / camLen;
        s_LastCamForward.y = camDy / camLen;
        s_LastCamRight.x   =  s_LastCamForward.y;
        s_LastCamRight.y   = -s_LastCamForward.x;
    }
    // Иначе — используем s_LastCamForward / s_LastCamRight как есть

    // Собираем вектор движения из ввода (относительно камеры)
    Vector3D targetDir = {0.0f, 0.0f, 0.0f};
    targetDir.x += s_LastCamForward.x * input.moveForward + s_LastCamRight.x * input.moveStrafe;
    targetDir.y += s_LastCamForward.y * input.moveForward + s_LastCamRight.y * input.moveStrafe;

    float dirLen = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
    bool isMoving = (dirLen > 0.01f);

    if (isMoving) {
        targetDir.x /= dirLen;
        targetDir.y /= dirLen;
    }

    // ── 5. Стамина ──
    bool canSprint = (m_CurrentStamina > 0.0f) && isMoving && (gs.playerMode == UnitMode::Scout);
    bool activeSprint = input.isSprinting && canSprint;

    if (activeSprint) {
        m_CurrentStamina = std::max(0.0f, m_CurrentStamina - m_StaminaDrain * dt);
    } else {
        m_CurrentStamina = std::min(m_MaxStamina, m_CurrentStamina + m_StaminaRegen * dt);
    }

    // ══════════════════════════════════════════════════════════
    // 6. Нырок (Dive — Helldivers 2)
    //
    // Нырок идёт В НАПРАВЛЕНИИ НАЖАТЫХ КЛАВИШ (targetDir).
    // Камера смотрит прямо, игрок жмёт D → кувырок вправо.
    // Камера смотрит прямо, игрок жмёт W → кувырок вперёд.
    // ══════════════════════════════════════════════════════════
    if (gs.playerMode == UnitMode::Scout) {
        if (input.isDiving && !m_IsDiving && isMoving) {
            m_IsDiving = true;
            m_DiveTimer = Config::PLAYER_DIVE_TIME;
            m_DiveDirection = targetDir;
            m_DiveDirection.z = 0.0f;
        }

        if (m_IsDiving) {
            m_DiveTimer -= dt;
            if (m_DiveTimer <= 0.0f) {
                m_IsDiving = false;
                gs.playerPos.z = 0.0f;
            } else {
                float nextX = gs.playerPos.x + m_DiveDirection.x * Config::PLAYER_DIVE_SPEED * dt;
                float nextY = gs.playerPos.y + m_DiveDirection.y * Config::PLAYER_DIVE_SPEED * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.playerPos.y, playerRadius))
                    gs.playerPos.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.playerPos.x, nextY, playerRadius))
                    gs.playerPos.y = nextY;

                float progress = 1.0f - (m_DiveTimer / Config::PLAYER_DIVE_TIME);
                gs.playerPos.z = std::sin(progress * 3.14159265f) * 0.5f;
            }
            return;  // Во время нырка WASD заблокировано
        }
    }

    // ── 7. Плавное движение с инерцией ──
    Vector3D targetVelocity = {0.0f, 0.0f, 0.0f};

    if (isMoving) {
        float currentMoveSpeed = activeSprint ? m_SprintSpeed : m_WalkSpeed;

        if (gs.playerMode == UnitMode::Scout && gs.isAiming)
            currentMoveSpeed *= 0.63f;

        if (gs.playerMode == UnitMode::Titan && gs.titan.systems.tracksCondition < 40.0f)
            currentMoveSpeed *= 0.3f;

        targetVelocity.x = targetDir.x * currentMoveSpeed;
        targetVelocity.y = targetDir.y * currentMoveSpeed;
    }

    float currentLerp = isMoving ? m_Acceleration : m_Deceleration;
    m_Velocity.x += (targetVelocity.x - m_Velocity.x) * currentLerp * dt;
    m_Velocity.y += (targetVelocity.y - m_Velocity.y) * currentLerp * dt;

    // ── 8. Коллизии ──
    float nextX = gs.playerPos.x + m_Velocity.x * dt;
    float nextY = gs.playerPos.y + m_Velocity.y * dt;

    if (!Collisions::checkWorldCollision(gs, nextX, gs.playerPos.y, playerRadius))
        gs.playerPos.x = nextX;
    if (!Collisions::checkWorldCollision(gs, gs.playerPos.x, nextY, playerRadius))
        gs.playerPos.y = nextY;

    // ── 9. Угол взгляда на курсор ──
    float dx = gs.mouseWorldPos.x - gs.playerPos.x;
    float dy = gs.mouseWorldPos.y - gs.playerPos.y;

    if (std::abs(dx) > 0.05f || std::abs(dy) > 0.05f) {
        m_FacingAngle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
        if (m_FacingAngle < 0.0f)
            m_FacingAngle += 360.0f;
    }
}

}  // namespace bunker
