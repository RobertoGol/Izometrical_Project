#include "content/MeshBuilder.hpp"
#include <SFML/OpenGL.hpp>
// #include <glad/glad.h> // Раскомментируй, когда подключишь загрузчик расширений OpenGL

namespace bunker {

    MeshComponent MeshBuilder::loadToGPU(const std::vector<Vertex>& vertices, 
                                         const std::vector<std::uint32_t>& indices, 
                                         std::uint32_t materialID) 
    {
        MeshComponent mesh;
        mesh.indexCount = static_cast<std::uint32_t>(indices.size());
        mesh.materialID = materialID;

        std::uint32_t vbo, ebo;

        // Генерируем буферы на видеокарте
        // glGenVertexArrays(1, &mesh.vaoID);
        // glGenBuffers(1, &vbo);
        // glGenBuffers(1, &ebo);

        // Биндим VAO (он запомнит все настройки атрибутов ниже)
        // glBindVertexArray(mesh.vaoID);

        // Заливаем вершины (VBO)
        // glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Заливаем индексы (EBO) - указывают, в каком порядке соединять вершины в треугольники
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);

        // Настраиваем layout для шейдера (сообщаем видеокарте, где лежат координаты, где нормали)
        
        // location = 0: Position (vec3)
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        // glEnableVertexAttribArray(0);

        // location = 1: Normal (vec3)
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // glEnableVertexAttribArray(1);

        // location = 2: TexCoords (vec2)
        // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        // glEnableVertexAttribArray(2);

        // Отвязываем VAO, чтобы случайно не сломать его при дальнейшей работе
        // glBindVertexArray(0);

        return mesh;
    }

    MeshComponent MeshBuilder::createTestCube() 
    {
        // 8 вершин обычного куба
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}}
        };

        // Индексы для 12 треугольников (по 2 на каждую из 6 граней)
        std::vector<std::uint32_t> indices = {
            0, 1, 2, 2, 3, 0, // Front
            1, 5, 6, 6, 2, 1, // Right
            7, 6, 5, 5, 4, 7, // Back
            4, 0, 3, 3, 7, 4, // Left
            4, 5, 1, 1, 0, 4, // Bottom
            3, 2, 6, 6, 7, 3  // Top
        };

        return loadToGPU(vertices, indices);
    }

} // namespace bunker