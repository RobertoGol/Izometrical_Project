#pragma once
#include "core/ECS.hpp"
#include "entities/Camera.hpp" // Твоя будущая 3D-камера

namespace bunker {

    class Renderer3D {
    public:
        Renderer3D() = default;
        ~Renderer3D() = default;

        void initialize();
        
        // Главная функция отрисовки кадра
        void renderScene(Registry& registry, const Camera& camera);

    private:
        std::uint32_t m_shaderProgram;
        
        // Кэшированные локации uniform-переменных в шейдере
        int m_locView;
        int m_locProjection;
        int m_locModel;

        void loadShaders();
        void bindMaterial(std::uint32_t materialID);
    };

} // namespace bunker