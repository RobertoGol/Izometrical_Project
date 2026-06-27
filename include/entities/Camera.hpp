#pragma once

#include <SFML/Graphics.hpp>
#include "Types.hpp"
#include "IsoMath.hpp"
#include "Constants.hpp"

namespace bunker {

class Camera {
private:
    sf::View  m_View;
    Vector3D  m_Target     = {5.0f, 5.0f, 0.0f};   // Точка фокуса в мировых координатах
    float     m_Zoom       = 1.0f;                   // Множитель масштаба
    float     m_SmoothSpeed = Config::CAMERA_SMOOTH_SPEED;

public:
    Camera() {
        m_View.setSize(static_cast<float>(Config::SCREEN_WIDTH),
                       static_cast<float>(Config::SCREEN_HEIGHT));
    }

    // ── Обновление камеры каждый кадр ──
    // playerPos   — текущая позиция игрока
    // mouseWorld  — мировые координаты курсора мыши
    // isAiming    — зажата ли правая кнопка (прицеливание)
    // dt          — deltaTime
    void update(const Vector3D& playerPos, const Vector3D& mouseWorld,
                bool isAiming, float dt)
    {
        Vector3D goal;

        if (isAiming) {
            // Смещение фокуса на 35% к курсору для тактического обзора (Helldivers style)
            goal.x = playerPos.x * (1.0f - Config::CAMERA_AIM_OFFSET)
                   + mouseWorld.x * Config::CAMERA_AIM_OFFSET;
            goal.y = playerPos.y * (1.0f - Config::CAMERA_AIM_OFFSET)
                   + mouseWorld.y * Config::CAMERA_AIM_OFFSET;
        } else {
            goal = playerPos;
        }

        // Плавная интерполяция к цели
        float t = IsoMath::clamp(m_SmoothSpeed * dt, 0.0f, 1.0f);
        m_Target = IsoMath::lerp(m_Target, goal, t);

        // Обновляем SFML View
        sf::Vector2f screenCenter = IsoMath::worldToScreen(m_Target);
        m_View.setCenter(screenCenter);
    }

    // ── Применить камеру к окну ──
    void applyTo(sf::RenderWindow& window) const {
        window.setView(m_View);
    }

    // ── Получить текущую цель камеры ──
    const Vector3D& getTarget() const { return m_Target; }

    // ── Перевести пиксельные координаты мыши в мировые через камеру ──
    Vector3D screenToWorld(const sf::RenderWindow& window, const sf::Vector2i& pixelPos) const {
        sf::Vector2f coords = window.mapPixelToCoords(pixelPos, m_View);
        return IsoMath::screenToWorld3D(coords.x, coords.y);
    }

    // ── Получить SFML View (для рендеринга HUD нужно сбрасывать) ──
    const sf::View& getView() const { return m_View; }

    // ── Установить зум ──
    void setZoom(float z) {
        m_Zoom = z;
        m_View.setSize(Config::SCREEN_WIDTH * m_Zoom,
                       Config::SCREEN_HEIGHT * m_Zoom);
    }

    float getZoom() const { return m_Zoom; }
};

}  // namespace bunker
