#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace bunker
{
    class MaterialSystem
    {
      public:
        void cacheUniforms(std::uint32_t shaderProgram);
        void bind(std::uint32_t materialID) const;

        static glm::vec3 debugColor(std::uint32_t materialID);

      private:
        std::uint32_t m_ShaderProgram = 0;
        int m_LocMaterialColor = -1;
        int m_LocMaterialEmissive = -1;
        int m_LocMaterialRoughness = -1;
        int m_LocMaterialMetallic = -1;
        int m_LocMaterialDebugColor = -1;
    };

} // namespace bunker
