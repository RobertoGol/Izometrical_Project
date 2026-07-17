#include "content/MeshBuilder.hpp"
#include <SFML/OpenGL.hpp>
#include <vector>
#include <cmath>

namespace bunker {

    MeshComponent MeshBuilder::loadToGPU(const std::vector<Vertex>& vertices, 
                                        const std::vector<std::uint32_t>& indices, 
                                        std::uint32_t materialID) 
    {
        MeshComponent mesh;
        mesh.indexCount = static_cast<std::uint32_t>(indices.size());
        mesh.materialID = materialID;
        
        // Здесь твой код инициализации буферов (glGenBuffers и т.д.)
        return mesh;
    }

    MeshComponent MeshBuilder::createGround(float size) {
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        
        int segments = 10; 
        float step = size / segments;

        for (int z = 0; z <= segments; ++z) {
            for (int x = 0; x <= segments; ++x) {
                float posX = x * step - size / 2.0f;
                float posZ = z * step - size / 2.0f;
                
                vertices.push_back({{posX, 0.0f, posZ}, {0.0f, 1.0f, 0.0f}, {(float)x/segments, (float)z/segments}});
            }
        }

        for (int z = 0; z < segments; ++z) {
            for (int x = 0; x < segments; ++x) {
                int row1 = z * (segments + 1);
                int row2 = (z + 1) * (segments + 1);
                
                indices.push_back(row1 + x);
                indices.push_back(row1 + x + 1);
                indices.push_back(row2 + x);

                indices.push_back(row1 + x + 1);
                indices.push_back(row2 + x + 1);
                indices.push_back(row2 + x);
            }
        }

        return loadToGPU(vertices, indices);
    } // <-- Теперь эта скобка на месте!

    MeshComponent MeshBuilder::createTestCube() 
    {
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

        std::vector<std::uint32_t> indices = {
            0, 1, 2, 2, 3, 0,
            1, 5, 6, 6, 2, 1,
            7, 6, 5, 5, 4, 7,
            4, 0, 3, 3, 7, 4,
            4, 5, 1, 1, 0, 4,
            3, 2, 6, 6, 7, 3
        };

        return loadToGPU(vertices, indices);
    } // <-- Эта скобка закрывает createTestCube

} // namespace bunker