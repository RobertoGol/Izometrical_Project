#pragma once
#include "core/ECS.hpp"
#include "entities/Camera.hpp"
#include "world/WeatherSystem.hpp"
#include <glm/glm.hpp>

namespace bunker {

    enum class RendererDebugOverlay
    {
        None,
        Normals,
        MaterialIds
    };

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
        void applyWeather(const WeatherRuntimeState& weather);
        void setDebugOverlay(RendererDebugOverlay overlay);
        RendererDebugOverlay debugOverlay() const { return m_debugOverlay; }

    private:
        std::uint32_t m_shaderProgram = 0;

        // Кэшированные локации uniform-переменных в шейдере
        int m_locView = -1;
        int m_locProjection = -1;
        int m_locModel = -1;
        int m_locMaterialColor = -1;
        int m_locMaterialEmissive = -1;
        int m_locMaterialRoughness = -1;
        int m_locMaterialMetallic = -1;
        int m_locLightDirection = -1;
        int m_locAmbientColor = -1;
        int m_locLightColor = -1;
        int m_locFogColor = -1;
        int m_locDebugOverlay = -1;
        int m_locMaterialDebugColor = -1;
        bool m_initialized = false;
        bool m_wireframeEnabled = false;
        RendererDebugOverlay m_debugOverlay = RendererDebugOverlay::None;
        glm::vec3 m_skyColor{0.10f, 0.11f, 0.14f};
        glm::vec3 m_lightDirection{-0.35f, 0.85f, 0.28f};
        glm::vec3 m_ambientColor{0.055f, 0.065f, 0.075f};
        glm::vec3 m_lightColor{0.76f, 0.68f, 0.52f};
        glm::vec3 m_fogColor{0.11f, 0.12f, 0.105f};

        void loadShaders();
        void bindMaterial(std::uint32_t materialID);
        static glm::vec3 materialDebugColor(std::uint32_t materialID);
    };

} // namespace bunker
