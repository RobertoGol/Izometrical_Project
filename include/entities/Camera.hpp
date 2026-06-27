#pragma once

#include <SFML/Graphics.hpp>
#include "core/Types.hpp"
#include "core/IsoMath.hpp"
#include "core/Constants.hpp"

namespace bunker
{

    class Camera
    {
    private:
        sf::View m_View;
        Vector3D m_Target = {5.0f, 5.0f, 0.0f};
        float m_Zoom = 1.0f;
        float m_SmoothSpeed = Config::CAMERA_SMOOTH_SPEED;

    public:
        Camera()
        {
            m_View.setSize(static_cast<float>(Config::SCREEN_WIDTH),
                           static_cast<float>(Config::SCREEN_HEIGHT));
        }

        void update(const Vector3D &playerPos, const Vector3D &mouseWorld, bool isAiming, float dt);

        void applyTo(sf::RenderWindow &window) const
        {
            window.setView(m_View);
        }

        const Vector3D &getTarget() const { return m_Target; }

        Vector3D screenToWorld(const sf::RenderWindow &window, const sf::Vector2i &pixelPos) const
        {
            sf::Vector2f coords = window.mapPixelToCoords(pixelPos, m_View);
            return IsoMath::screenToWorld3D(coords.x, coords.y);
        }

        const sf::View &getView() const { return m_View; }

        void setZoom(float z)
        {
            m_Zoom = z;
            m_View.setSize(Config::SCREEN_WIDTH * m_Zoom,
                           Config::SCREEN_HEIGHT * m_Zoom);
        }

        float getZoom() const { return m_Zoom; }
    };

} // namespace bunker
