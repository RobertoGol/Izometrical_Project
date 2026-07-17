#include "render/Renderer3D.hpp"
#include <SFML/OpenGL.hpp>
// Раскомментируй строку ниже, если у тебя настроен GLAD
// #include <glad/glad.h>

namespace bunker {

    void Renderer3D::initialize() {
        loadShaders();
        
        // Глобальные настройки стейта OpenGL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS); // Рисуем только то, что ближе к камере
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void Renderer3D::renderScene(Registry& registry, const Camera& camera) {
    // 1. Очистка буферов
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    2. Небо
    renderSkyDome();

    // 3. --- ДЕБАГ: ВКЛЮЧАЕМ СЕТКУ (Wireframe) ---
    // Это заставит OpenGL рисовать контуры треугольников (Wireframe),
    // даже если шейдеры еще не настроены или текстуры не привязаны.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 4. Отрисовка ECS объектов
    auto& transforms = registry.transforms.getRawData();
    auto& meshes = registry.meshes.getRawData();;

    for (size_t i = 0; i < meshes.size(); ++i) {
        const auto& mesh = meshes[i];
        
        // РАЗКОММЕНТИРУЙ ЭТИ СТРОКИ:
        glBindVertexArray(mesh.vaoID);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
    }
    
    // Возвращаем режим заливки, чтобы UI не превратился в сетку
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
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