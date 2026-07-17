#include "entities/Camera.hpp"
#include "core/Constants.hpp" // Для Config::SCREEN_WIDTH/HEIGHT
#include <SFML/Window/Mouse.hpp>
#include <algorithm> // Для std::clamp

namespace bunker {

    Camera::Camera()
        : m_position(glm::vec3(0.0f, 2.0f, 5.0f)) // Высота глаз игрока ~2.0f
        , m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f))
        , m_yaw(-90.0f) // Смотрим вдоль оси -Z по умолчанию
        , m_pitch(0.0f)
        , m_movementSpeed(15.0f) // Скорость ходьбы
        , m_mouseSensitivity(0.1f)
        , m_fov(45.0f)
        , m_firstMouse(true)
    {
        updateCameraVectors();
    }

    void Camera::update(float dt, sf::Window& window) {
        // 1. ОБРАБОТКА ВРАЩЕНИЯ МЫШЬЮ
        if (window.hasFocus()) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            
            if (m_firstMouse) {
                m_lastMouseX = mousePos.x;
                m_lastMouseY = mousePos.y;
                m_firstMouse = false;
            }

            float xoffset = mousePos.x - m_lastMouseX;
            float yoffset = m_lastMouseY - mousePos.y; // В оконных координатах Y идет вниз, а в 3D - вверх
            
            m_lastMouseX = mousePos.x;
            m_lastMouseY = mousePos.y;

            xoffset *= m_mouseSensitivity;
            yoffset *= m_mouseSensitivity;

            m_yaw   += xoffset;
            m_pitch += yoffset;

            // Ограничиваем наклон, чтобы шея не "сломалась" (нельзя смотреть за спину через верх)
            m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

            updateCameraVectors();

            // Опционально: возвращаем курсор в центр экрана, чтобы он не упирался в края окна
            // sf::Mouse::setPosition(sf::Vector2i(Config::SCREEN_WIDTH / 2, Config::SCREEN_HEIGHT / 2), window);
            // m_lastMouseX = Config::SCREEN_WIDTH / 2;
            // m_lastMouseY = Config::SCREEN_HEIGHT / 2;
        }

        // 2. ОБРАБОТКА ДВИЖЕНИЯ WASD
        float velocity = m_movementSpeed * dt;
        
        // ВАЖНО: В реальном проекте это движение должно управляться PlayerController через физику,
        // но для базового свободного полета камеры оставим так.
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            m_position += m_front * velocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            m_position -= m_front * velocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            m_position -= m_right * velocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            m_position += m_right * velocity;
    }

    glm::mat4 Camera::getViewMatrix() const {
        // Матрица Вида = LookAt(откуда смотрим, куда смотрим, где верх)
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const {
        // Матрица Проекции = Перспектива (FOV, соотношение сторон, ближняя плоскость отсечения, дальняя)
        return glm::perspective(glm::radians(m_fov), aspectRatio, nearPlane, farPlane);
    }

    void Camera::updateCameraVectors() {
        // Вычисляем новый вектор front на основе углов Эйлера
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        
        m_front = glm::normalize(front);
        
        // Пересчитываем векторы right и up (векторное произведение)
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));  
        m_up    = glm::normalize(glm::cross(m_right, m_front));
    }

} // namespace bunker