#include "render/Renderer3D.hpp"
#include "core/Constants.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace bunker {

    void Renderer3D::initialize() {
        static bool gladLoaded = false;
        if (!gladLoaded) {
            gladLoaded = gladLoadGL() != 0;
        }
        if (!gladLoaded) {
            m_initialized = false;
            return;
        }

        loadShaders();

        // Глобальные настройки стейта OpenGL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS); // Рисуем только то, что ближе к камере

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        m_initialized = true;
    }

    void Renderer3D::renderScene(Registry& registry, const Camera& camera) {
        if (!m_initialized) {
            return;
        }

        // 1. Очистка буферов
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Небо
        renderSkyDome();

        // 3. --- ДЕБАГ: ВКЛЮЧАЕМ СЕТКУ (Wireframe) ---
        // Это заставит OpenGL рисовать контуры треугольников (Wireframe),
        // даже если шейдеры еще не настроены или текстуры не привязаны.
        glPolygonMode(GL_FRONT_AND_BACK, m_wireframeEnabled ? GL_LINE : GL_FILL);

        if (m_shaderProgram != 0) {
            const float aspectRatio = static_cast<float>(Config::SCREEN_WIDTH) / static_cast<float>(Config::SCREEN_HEIGHT);
            const glm::mat4 view = camera.getViewMatrix();
            const glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

            glUseProgram(m_shaderProgram);
            if (m_locView >= 0) {
                glUniformMatrix4fv(m_locView, 1, GL_FALSE, glm::value_ptr(view));
            }
            if (m_locProjection >= 0) {
                glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(projection));
            }
        }

        // 4. Отрисовка ECS объектов
        auto& transforms = registry.transforms.getRawData();
        auto& meshes = registry.meshes.getRawData();
        const auto drawCount = std::min(meshes.size(), transforms.size());

        for (size_t i = 0; i < drawCount; ++i) {
            const auto& mesh = meshes[i];
            const auto& transform = transforms[i];

            if (m_shaderProgram != 0 && m_locModel >= 0) {
                glm::mat4 model{1.0f};
                model = glm::translate(model, transform.position);
                model *= glm::mat4_cast(transform.rotation);
                model = glm::scale(model, transform.scale);
                glUniformMatrix4fv(m_locModel, 1, GL_FALSE, glm::value_ptr(model));
            }

            // РАЗКОММЕНТИРУЙ ЭТИ СТРОКИ:
            glBindVertexArray(mesh.vaoID);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }

        // Возвращаем режим заливки, чтобы UI не превратился в сетку
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Renderer3D::setWireframeEnabled(bool enabled) {
        m_wireframeEnabled = enabled;
    }

    void Renderer3D::renderSkyDome() {
        // Отключаем запись в буфер глубины, чтобы небо не перекрывало объекты
        glDepthMask(GL_FALSE);

        // glUseProgram(m_SkyShader);
        // glBindTexture(GL_TEXTURE_2D, m_SkyTextureID);

        // m_SkyMesh.draw();

        // Возвращаем запись глубины
        glDepthMask(GL_TRUE);
    }

    void Renderer3D::loadShaders() {
        // Тут будет загрузка шейдеров: glCreateShader, glCompileShader и т.д.
        // Shader loading
    }

    void Renderer3D::bindMaterial(std::uint32_t materialID) {
        // Тут будет glBindTexture для текстур (Diffuse, Normal, etc.)
        // Material binding
    }

} // namespace bunker
