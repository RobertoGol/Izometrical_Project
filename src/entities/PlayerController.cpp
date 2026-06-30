#include "PlayerController.hpp"
#include "Collisions.hpp"
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

        // ══════════════════════════════════════════════════════════
        // 1. РАДИУС ХИТБОКСА
        //    Scout = 0.25, Titan(Танк) = 0.55
        // ══════════════════════════════════════════════════════════
        float playerRadius = (gs.playerMode == UnitMode::Titan)
                                 ? Config::TITAN_RADIUS
                                 : Config::PLAYER_RADIUS;

        // ══════════════════════════════════════════════════════════
        // 2. ПРИЦЕЛИВАНИЕ (ПКМ)
        // ══════════════════════════════════════════════════════════
        gs.isAiming = input.isAiming;

        // ══════════════════════════════════════════════════════════
        // 3. КУЛДАУН СТРЕЛЬБЫ
        // ══════════════════════════════════════════════════════════
        if (gs.fireCooldown > 0.0f)
            gs.fireCooldown -= dt;

        // ══════════════════════════════════════════════════════════
        // 4. ОСИ КАМЕРЫ (направление от игрока к курсору мыши)
        //
        //    camForward = нормализованный вектор игрок → мышь
        //    camRight   = перпендикуляр вправо (поворот 90° CW)
        //
        //    Если мышь на игроке (camLen ≈ 0) — оси НЕ обнуляются,
        //    используются последние запомненные. Т.е. куда камера
        //    смотрела раньше — туда и продолжает.
        // ══════════════════════════════════════════════════════════
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
        // Иначе — s_LastCamForward и s_LastCamRight остаются как были

        // ══════════════════════════════════════════════════════════
        // 5. ВЕКТОР ДВИЖЕНИЯ ИЗ ВВОДА (относительно камеры)
        //
        //    W (+moveForward) = в сторону курсора
        //    S (-moveForward) = от курсора
        //    D (+moveStrafe)  = вправо от направления камеры
        //    A (-moveStrafe)  = влево от направления камеры
        //
        //    Диагональ нормализуется → нет x1.41 скорости.
        // ══════════════════════════════════════════════════════════
        Vector3D targetDir = {0.0f, 0.0f, 0.0f};
        targetDir.x = s_LastCamForward.x * input.moveForward + s_LastCamRight.x * input.moveStrafe;
        targetDir.y = s_LastCamForward.y * input.moveForward + s_LastCamRight.y * input.moveStrafe;

        float dirLen = std::sqrt(targetDir.x * targetDir.x + targetDir.y * targetDir.y);
        bool isMoving = (dirLen > 0.01f);

        if (isMoving)
        {
            targetDir.x /= dirLen;
            targetDir.y /= dirLen;
        }

        // ══════════════════════════════════════════════════════════
        // 6. СТАМИНА (AP — Action Points, Log Horizon стиль)
        //
        //    Спринт (Shift) тратит стамину.
        //    Без спринта — стамина восстанавливается.
        //    Спринт доступен только Scout.
        // ══════════════════════════════════════════════════════════
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

        // ══════════════════════════════════════════════════════════
        // 7. НЫРОК (Dive — Helldivers 2 стиль)
        //
        //    Space + движение = кувырок В НАПРАВЛЕНИИ НАЖАТЫХ КЛАВИШ.
        //
        //    Пример: камера смотрит вперёд, игрок жмёт D + Space →
        //    кувырок ВПРАВО (не вперёд!).
        //
        //    Во время нырка обычное WASD заблокировано.
        //    Есть парабола по Z для визуального подъёма тела.
        //    Коллизии со стенами работают и в полёте.
        //    Только для Scout (не Танк).
        // ══════════════════════════════════════════════════════════
        if (gs.playerMode == UnitMode::Scout)
        {
            // Начало нырка — Space зажат, не уже в нырке, есть движение
            if (input.isDiving && !m_IsDiving && isMoving)
            {
                m_IsDiving = true;
                m_DiveTimer = Config::PLAYER_DIVE_TIME;
                // Направление нырка = куда нажимает игрок (targetDir), уже нормализован
                m_DiveDirection = targetDir;
                m_DiveDirection.z = 0.0f;
            }

            // Обработка полёта в нырке
            if (m_IsDiving)
            {
                m_DiveTimer -= dt;

                if (m_DiveTimer <= 0.0f)
                {
                    // Приземление
                    m_IsDiving = false;
                    gs.playerPos.z = 0.0f;
                }
                else
                {
                    // Движение в направлении нырка
                    float nextX = gs.playerPos.x + m_DiveDirection.x * Config::PLAYER_DIVE_SPEED * dt;
                    float nextY = gs.playerPos.y + m_DiveDirection.y * Config::PLAYER_DIVE_SPEED * dt;

                    // Скользящие коллизии по осям
                    if (!Collisions::checkWorldCollision(gs, nextX, gs.playerPos.y, playerRadius))
                        gs.playerPos.x = nextX;
                    if (!Collisions::checkWorldCollision(gs, gs.playerPos.x, nextY, playerRadius))
                        gs.playerPos.y = nextY;

                    // Визуальная парабола подъёма тела (sin-кривая по Z)
                    float progress = 1.0f - (m_DiveTimer / Config::PLAYER_DIVE_TIME);
                    gs.playerPos.z = std::sin(progress * 3.14159265f) * 0.5f;
                }

                // Во время нырка стандартное WASD заблокировано — выходим
                return;
            }
        }

        // ══════════════════════════════════════════════════════════
        // 8. ПЛАВНОЕ ДВИЖЕНИЕ С ИНЕРЦИЕЙ
        //
        //    targetVelocity = желаемая скорость (направление * speed)
        //    m_Velocity     = текущая скорость (сглаженная lerp-ом)
        //
        //    Разгон (m_Acceleration) быстрый, торможение (m_Deceleration)
        //    тоже заметное — убирает "деревянность" движения.
        //
        //    Штрафы скорости:
        //    - Прицеливание (ПКМ): 63% от базовой
        //    - Повреждённые гусеницы Танка (<40%): 30% от базовой
        // ══════════════════════════════════════════════════════════
        Vector3D targetVelocity = {0.0f, 0.0f, 0.0f};

        if (isMoving)
        {
            float currentMoveSpeed = activeSprint ? m_SprintSpeed : m_WalkSpeed;

            // Штраф при прицеливании
            if (gs.playerMode == UnitMode::Scout && gs.isAiming)
                currentMoveSpeed *= 0.63f;

            // Штраф при повреждённых гусеницах Танка
            if (gs.playerMode == UnitMode::Titan && gs.titan.systems.tracksCondition < 40.0f)
                currentMoveSpeed *= 0.3f;

            targetVelocity.x = targetDir.x * currentMoveSpeed;
            targetVelocity.y = targetDir.y * currentMoveSpeed;
        }

        // Lerp к целевой скорости
        float currentLerp = isMoving ? m_Acceleration : m_Deceleration;
        m_Velocity.x += (targetVelocity.x - m_Velocity.x) * currentLerp * dt;
        m_Velocity.y += (targetVelocity.y - m_Velocity.y) * currentLerp * dt;

        // ══════════════════════════════════════════════════════════
        // 9. КОЛЛИЗИИ СО СТЕНАМИ (скользящий обсчёт по осям)
        //
        //    Проверяем X и Y раздельно — если X заблокирован стеной,
        //    Y всё равно может пройти. Это позволяет игроку
        //    "скользить" вдоль стен при диагональном движении.
        // ══════════════════════════════════════════════════════════
        float nextX = gs.playerPos.x + m_Velocity.x * dt;
        float nextY = gs.playerPos.y + m_Velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, gs.playerPos.y, playerRadius))
            gs.playerPos.x = nextX;
        if (!Collisions::checkWorldCollision(gs, gs.playerPos.x, nextY, playerRadius))
            gs.playerPos.y = nextY;

        // ══════════════════════════════════════════════════════════
        // 10. УГОЛ ВЗГЛЯДА НА КУРСОР МЫШИ
        //
        //     atan2 → градусы 0..360.
        //     Мёртвая зона: если мышь слишком близко к игроку —
        //     сохраняем прошлый угол (чтобы спрайт не дёргался).
        // ══════════════════════════════════════════════════════════
        float dx = gs.mouseWorldPos.x - gs.playerPos.x;
        float dy = gs.mouseWorldPos.y - gs.playerPos.y;

        if (std::abs(dx) > 0.05f || std::abs(dy) > 0.05f)
        {
            m_FacingAngle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
            if (m_FacingAngle < 0.0f)
                m_FacingAngle += 360.0f;
        }
    }

} // namespace bunker