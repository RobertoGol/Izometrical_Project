#pragma once
#include "core/ECS.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace bunker {

    // Формат одной вершины в памяти.
    // Строго 32 байта: 3 floats (pos) + 3 floats (normal) + 2 floats (uv)
    // Идеально ложится в кэш-линию.
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    class MeshBuilder {
    public:
        // Загружает сырые данные на видеокарту и возвращает компонент для ECS
        static MeshComponent loadToGPU(const std::vector<Vertex>& vertices, 
                                       const std::vector<std::uint32_t>& indices, 
                                       std::uint32_t materialID = 0);

        // Для быстрого теста создадим генератор примитивного 3D-куба
        static MeshComponent createTestCube();
    };

} // namespace bunker