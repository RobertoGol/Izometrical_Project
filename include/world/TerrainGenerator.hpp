    #pragma once
    #include <vector>
    #include "content/MeshBuilder.hpp"

    namespace bunker {

        struct TerrainParams {
            float size = 50.0f;
            int segments = 50;
            float heightScale = 3.0f;
            float frequency = 0.1f;
        };

        class TerrainGenerator {
        public:
            // Автоматически генерирует всю сетку с рельефом
            static MeshComponent generateAutoTerrain(const TerrainParams& params);
        };

    }