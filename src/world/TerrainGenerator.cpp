#include "world/TerrainGenerator.hpp"
#include <cmath>

namespace bunker {

    MeshComponent TerrainGenerator::generateAutoTerrain(const TerrainParams& params) {
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        
        float step = params.size / params.segments;

        for (int z = 0; z <= params.segments; ++z) {
            for (int x = 0; x <= params.segments; ++x) {
                float posX = x * step - params.size / 2.0f;
                float posZ = z * step - params.size / 2.0f;
                
                // --- АВТО-ГЕНЕРАТОР РЕЛЬЕФА (Noise) ---
                // Сумма синусов создает подобие холмистого ландшафта
                float posY = (std::sin(x * params.frequency) * std::cos(z * params.frequency) * params.heightScale) +
                             (std::sin(x * params.frequency * 2.0f) * 0.5f);

                vertices.push_back({
                    {posX, posY, posZ}, 
                    {0.0f, 1.0f, 0.0f}, 
                    {(float)x/params.segments, (float)z/params.segments}
                });
            }
        }

        // Индексы
        for (int z = 0; z < params.segments; ++z) {
            for (int x = 0; x < params.segments; ++x) {
                int row1 = z * (params.segments + 1);
                int row2 = (z + 1) * (params.segments + 1);
                
                indices.push_back(row1 + x);
                indices.push_back(row1 + x + 1);
                indices.push_back(row2 + x);

                indices.push_back(row1 + x + 1);
                indices.push_back(row2 + x + 1);
                indices.push_back(row2 + x);
            }
        }

        return MeshBuilder::loadToGPU(vertices, indices);
    }
}