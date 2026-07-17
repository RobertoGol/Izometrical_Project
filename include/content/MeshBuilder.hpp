#pragma once
#include "core/ECS.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace bunker {

    // Формат одной вершины в памяти.
    // Строго 32 байта: 3 floats (pos) + 3 floats (normal) + 2 floats (uv)
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    class MeshBuilder {
    public:
        static MeshComponent loadToGPU(const std::vector<Vertex>& vertices, 
            const std::vector<std::uint32_t>& indices, 
            std::uint32_t materialID = 0);

        static MeshComponent createTestCube();
        
        // Добавь только эту строку:
        static MeshComponent createGround(float size);
    };

} // namespace bunker