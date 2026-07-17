#include "content/MeshBuilder.hpp"
#include "content/MaterialCatalog.hpp"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <cstddef>

namespace bunker {

    MeshComponent MeshBuilder::loadToGPU(const std::vector<Vertex>& vertices,
                                        const std::vector<std::uint32_t>& indices,
                                        std::uint32_t materialID)
    {
        MeshComponent mesh;
        mesh.indexCount = static_cast<std::uint32_t>(indices.size());
        mesh.materialID = materialID;

        if (vertices.empty() || indices.empty()) {
            return mesh;
        }

        if (!GLAD_GL_VERSION_3_0) {
            return mesh;
        }

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
            vertices.data(),
            GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
            indices.data(),
            GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, position)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, texCoords)));

        glBindVertexArray(0);

        mesh.vaoID = vao;

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

        return loadToGPU(vertices, indices, Material_Grass);
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

        return loadToGPU(vertices, indices, Material_RustMetal);
    } // <-- Эта скобка закрывает createTestCube

} // namespace bunker
