#include "render/MaterialSystem.hpp"
#include "content/MaterialCatalog.hpp"

#include <glad/glad.h>

#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace bunker
{
    void MaterialSystem::cacheUniforms(std::uint32_t shaderProgram)
    {
        m_ShaderProgram = shaderProgram;
        m_LocMaterialColor = glGetUniformLocation(m_ShaderProgram, "u_materialColor");
        m_LocMaterialEmissive = glGetUniformLocation(m_ShaderProgram, "u_materialEmissive");
        m_LocMaterialRoughness = glGetUniformLocation(m_ShaderProgram, "u_materialRoughness");
        m_LocMaterialMetallic = glGetUniformLocation(m_ShaderProgram, "u_materialMetallic");
        m_LocMaterialDebugColor = glGetUniformLocation(m_ShaderProgram, "u_materialDebugColor");
    }

    void MaterialSystem::bind(std::uint32_t materialID) const
    {
        if (m_ShaderProgram == 0 || m_LocMaterialColor < 0)
        {
            return;
        }

        const auto& material = getMaterial(materialID);
        const glm::vec3 color = hexToLinearRgb(material.hex);
        glUniform3fv(m_LocMaterialColor, 1, glm::value_ptr(color));
        if (m_LocMaterialDebugColor >= 0)
        {
            const glm::vec3 materialDebugColor = debugColor(materialID);
            glUniform3fv(m_LocMaterialDebugColor, 1, glm::value_ptr(materialDebugColor));
        }
        if (m_LocMaterialEmissive >= 0)
        {
            const glm::vec3 emissive = hexToLinearRgb(material.emissiveHex);
            glUniform3fv(m_LocMaterialEmissive, 1, glm::value_ptr(emissive));
        }
        if (m_LocMaterialRoughness >= 0)
        {
            glUniform1f(m_LocMaterialRoughness, material.roughness);
        }
        if (m_LocMaterialMetallic >= 0)
        {
            glUniform1f(m_LocMaterialMetallic, material.metallic);
        }
    }

    glm::vec3 MaterialSystem::debugColor(std::uint32_t materialID)
    {
        const std::uint32_t hash = materialID * 2654435761u;
        const float r = static_cast<float>((hash >> 16) & 0xFFu) / 255.0f;
        const float g = static_cast<float>((hash >> 8) & 0xFFu) / 255.0f;
        const float b = static_cast<float>(hash & 0xFFu) / 255.0f;
        return glm::clamp(glm::vec3{r, g, b} * 0.75f + glm::vec3{0.20f}, glm::vec3{0.0f}, glm::vec3{1.0f});
    }

} // namespace bunker
