#include "render/Renderer3D.hpp"

// Подключаем сырой OpenGL. 
// SFML предоставляет контекст, но 3D мы рисуем аппаратно.
#include <SFML/OpenGL.hpp> 

namespace bunker {

    void Renderer3D::initialize() {
        // Здесь будет инициализация PBR шейдеров (Albedo, Normal Map, Metallic/Roughness)
        loadShaders();
        
        // Глобальные настройки стейта OpenGL для 3D
        glEnable(GL_DEPTH_TEST); // Ближние объекты перекрывают дальние
        glEnable(GL_CULL_FACE);  // Не рисуем задние невидимые грани полигонов (оптимизация)
        glCullFace(GL_BACK);
    }

    void Renderer3D::renderScene(Registry& registry, const Camera& camera) {
        // 1. Очистка буферов перед новым кадром (Цвет + Глубина)
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // Фоновый цвет (атмосфера Пустоши)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Биндим главный PBR-шейдер
        // glUseProgram(m_shaderProgram);

        // 3. Установка стейта камеры
        // float viewMatrix[16];
        // float projMatrix[16];
        // camera.getViewMatrix(viewMatrix);
        // camera.getProjectionMatrix(projMatrix, 16.0f / 9.0f, 0.1f, 2000.0f);
        // glUniformMatrix4fv(m_locView, 1, GL_FALSE, viewMatrix);
        // glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, projMatrix);

        // 4. СВЕРХБЫСТРАЯ ИТЕРАЦИЯ ПО ECS
        // Данные лежат плотно. Процессор линейно читает кэш без прыжков памяти.
        auto& transforms = registry.transforms.getRawData();
        auto& meshes = registry.meshes.getRawData();

        for (size_t i = 0; i < meshes.size(); ++i) {
            const auto& mesh = meshes[i];
            const auto& transform = transforms[i]; 

            // Вычисление Model-матрицы из transform.position / rotation
            // float modelMatrix[16];
            // calculateModelMatrix(transform, modelMatrix);
            // glUniformMatrix4fv(m_locModel, 1, GL_FALSE, modelMatrix);

            // Биндим текстуры (PBR материалы)
            bindMaterial(mesh.materialID);

            // Аппаратный вызов отрисовки геометрии
            // glBindVertexArray(mesh.vaoID);
            // glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    void Renderer3D::loadShaders() {
        // Загрузка и компиляция вершинного и фрагментного шейдеров
    }

    void Renderer3D::bindMaterial(std::uint32_t materialID) {
        // Привязка текстур (Diffuse, Normal, ORM) к слотам видеокарты
    }

} // namespace bunker