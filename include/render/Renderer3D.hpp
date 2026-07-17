#pragma once
#include "core/ECS.hpp"
#include "entities/Camera.hpp"

namespace bunker {

    class Renderer3D {
    public:
        Renderer3D() = default;
        ~Renderer3D() = default;

        void initialize();

        // Главная функция отрисовки кадра
        void renderScene(Registry& registry, const Camera& camera);

        // --- ДОБАВЬ ЭТУ СТРОКУ ---
        void renderSkyDome();
        void setWireframeEnabled(bool enabled);

    private:
        std::uint32_t m_shaderProgram = 0;

        // Кэшированные локации uniform-переменных в шейдере
        int m_locView = -1;
        int m_locProjection = -1;
        int m_locModel = -1;
        bool m_initialized = false;
        bool m_wireframeEnabled = false;

        void loadShaders();
        void bindMaterial(std::uint32_t materialID);
    };

} // namespace bunker
