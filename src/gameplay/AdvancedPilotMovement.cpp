#include "gameplay/AdvancedPilotMovement.hpp"
#include "core/Constants.hpp"
#include <iostream>
#include <algorithm>

namespace bunker
{

    void AdvancedPilotMovementController::triggerKneeSlide(GameState &gs)
    {
        if (m_Mode == PilotLocomotionMode::GroundRunning)
        {
            m_Mode = PilotLocomotionMode::FloorSlideHop;
            m_SlideTimer = 1.25f;    // Длительность скольжения на коленях по полу
            gs.playerSpeed *= 1.45f; // Импульс ускорения подката!
            std::cout << "[LOCOMotion] Активировано тактическое скольжение по полу (Подкат)!" << std::endl;
        }
    }

    void AdvancedPilotMovementController::fireGrappleZipline(GameState &gs, const Vector3D &targetAnchor)
    {
        m_Grapple.isAttached = true;
        m_Grapple.anchorPoint = targetAnchor;
        float dx = targetAnchor.x - gs.playerPos.x;
        float dy = targetAnchor.y - gs.playerPos.y;
        m_Grapple.cableLength = std::sqrt(dx * dx + dy * dy);
        m_Mode = PilotLocomotionMode::GrappleZipline;
        std::cout << "[ZIPLINE] !! ТАРЗАНКА !! Выпущен тросовый крюк-кошка к координате ("
                  << targetAnchor.x << ", " << targetAnchor.y << ")!" << std::endl;
    }

    void AdvancedPilotMovementController::detachGrapple()
    {
        m_Grapple.isAttached = false;
        m_Mode = PilotLocomotionMode::GroundRunning;
    }

    void AdvancedPilotMovementController::updateLocomotionPhysics(GameState &gs, const WorldGridState &grid, float dt)
    {
        if (m_Mode == PilotLocomotionMode::FloorSlideHop)
        {
            m_SlideTimer -= dt;
            if (m_SlideTimer <= 0.0f)
            {
                m_Mode = PilotLocomotionMode::GroundRunning;
            }
        }
        else if (m_Mode == PilotLocomotionMode::GrappleZipline && m_Grapple.isAttached)
        {
            // Физика подтягивания Пилота по тарзанке на огромной скорости
            float pullSpeed = Config::PLAYER_WALK_SPEED * 3.5f;
            float dx = m_Grapple.anchorPoint.x - gs.playerPos.x;
            float dy = m_Grapple.anchorPoint.y - gs.playerPos.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist <= 0.5f)
            {
                detachGrapple(); // Достигли точки крепления троса
            }
            else
            {
                gs.playerPos.x += (dx / dist) * pullSpeed * dt;
                gs.playerPos.y += (dy / dist) * pullSpeed * dt;
            }
            return;
        }

        // Проверка прилипания к стенам для паркура (Wall running)
        int tx = static_cast<int>(gs.playerPos.x);
        int ty = static_cast<int>(gs.playerPos.y);
        if (tx > 0 && tx < Config::MAP_WIDTH - 1 && ty > 0 && ty < Config::MAP_HEIGHT - 1)
        {
            if (grid.sectorMap[tx - 1][ty] == 1 || grid.sectorMap[tx + 1][ty] == 1)
            {
                if (gs.playerSpeed > Config::PLAYER_WALK_SPEED * 1.1f)
                {
                    m_Mode = PilotLocomotionMode::WallRunningLeft; // Скольжение вдоль стены!
                }
            }
        }
    }

} // namespace bunker
