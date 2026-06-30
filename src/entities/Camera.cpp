#include "entities/Camera.hpp"
#include "core/IsoMath.hpp"
#include "core/Constants.hpp"
#include <algorithm>

namespace bunker
{

    void Camera::update(const Vector3D &playerPos, const Vector3D &mouseWorld,
                        bool isAiming, float dt)
    {
        Vector3D goal;

        if (isAiming)
        {
            goal.x = playerPos.x * (1.0f - Config::CAMERA_AIM_OFFSET) + mouseWorld.x * Config::CAMERA_AIM_OFFSET;
            goal.y = playerPos.y * (1.0f - Config::CAMERA_AIM_OFFSET) + mouseWorld.y * Config::CAMERA_AIM_OFFSET;
        }
        else
        {
            goal = playerPos;
        }

        float t = IsoMath::clamp(m_SmoothSpeed * dt, 0.0f, 1.0f);
        m_Target = IsoMath::lerp(m_Target, goal, t);

        sf::Vector2f screenCenter = IsoMath::worldToScreen(m_Target);
        m_View.setCenter(screenCenter);
    }

} // namespace bunker