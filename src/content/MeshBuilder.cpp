#include "content/MeshBuilder.hpp"
#include "content/MaterialCatalog.hpp"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <cstddef>

namespace bunker {
    namespace {
        void computeTangents(std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices) {
            for (auto& vertex : vertices) {
                vertex.tangent = {0.0f, 0.0f, 0.0f};
            }

            for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
                Vertex& v0 = vertices[indices[i]];
                Vertex& v1 = vertices[indices[i + 1]];
                Vertex& v2 = vertices[indices[i + 2]];

                const glm::vec3 edge1 = v1.position - v0.position;
                const glm::vec3 edge2 = v2.position - v0.position;
                const glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
                const glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;
                const float determinant = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;

                if (std::abs(determinant) < 0.000001f) {
                    continue;
                }

                const float invDeterminant = 1.0f / determinant;
                const glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * invDeterminant;
                v0.tangent += tangent;
                v1.tangent += tangent;
                v2.tangent += tangent;
            }

            for (auto& vertex : vertices) {
                if (glm::dot(vertex.tangent, vertex.tangent) < 0.000001f) {
                    vertex.tangent = {1.0f, 0.0f, 0.0f};
                } else {
                    vertex.tangent = glm::normalize(vertex.tangent);
                }
            }
        }
    }

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

        std::vector<Vertex> gpuVertices = vertices;
        computeTangents(gpuVertices, indices);

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
            static_cast<GLsizeiptr>(gpuVertices.size() * sizeof(Vertex)),
            gpuVertices.data(),
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

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, tangent)));

        glBindVertexArray(0);

        mesh.vaoID = vao;
        mesh.vboID = vbo;
        mesh.eboID = ebo;

        return mesh;
    }

    void MeshBuilder::releaseFromGPU(MeshComponent& mesh)
    {
        if (mesh.eboID != 0) {
            glDeleteBuffers(1, &mesh.eboID);
            mesh.eboID = 0;
        }
        if (mesh.vboID != 0) {
            glDeleteBuffers(1, &mesh.vboID);
            mesh.vboID = 0;
        }
        if (mesh.vaoID != 0) {
            glDeleteVertexArrays(1, &mesh.vaoID);
            mesh.vaoID = 0;
        }
        mesh.indexCount = 0;
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
