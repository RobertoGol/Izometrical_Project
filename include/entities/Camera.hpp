#pragma once
#include "core/ECS.hpp" // Для TransformComponent
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Window/Window.hpp> // <- Добавь это вместо простого предварительного объявления sf::Window

namespace bunker {

    class Camera {
    public:
        Camera();

        // Обновление физики камеры (движение WASD и вращение мышью)
        void update(float dt, sf::Window& window);

        // Получение матриц для отправки в шейдер
        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 2000.0f) const;

        // Позиция камеры в 3D мире
        glm::vec3 getPosition() const { return m_position; }
        
        // Вектор направления взгляда
        glm::vec3 getFront() const { return m_front; }

    private:
        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        glm::vec3 m_worldUp;

        // Углы Эйлера
        float m_yaw;
        float m_pitch;

        // Настройки управления
        float m_movementSpeed;
        float m_mouseSensitivity;
        float m_fov;

        // Состояние мыши для расчета дельты
        bool m_firstMouse;
        int m_lastMouseX;
        int m_lastMouseY;

        void updateCameraVectors();
    };

} // namespace bunker