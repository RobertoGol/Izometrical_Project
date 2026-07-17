#pragma once

#include <array>
#include <cstdint>

#include <glm/glm.hpp>

namespace bunker {

    enum MaterialId : std::uint32_t {
        Material_Default = 0,
        Material_Grass = 1,
        Material_Concrete = 2,
        Material_RustMetal = 3,
        Material_Dirt = 4
    };

    struct MaterialDefinition {
        std::uint32_t id = Material_Default;
        const char* name = "default";
        std::uint32_t hex = 0x8C8264;
    };

    inline glm::vec3 hexToLinearRgb(std::uint32_t hex) {
        const float r = static_cast<float>((hex >> 16) & 0xFF) / 255.0f;
        const float g = static_cast<float>((hex >> 8) & 0xFF) / 255.0f;
        const float b = static_cast<float>(hex & 0xFF) / 255.0f;
        return {r, g, b};
    }

    inline const std::array<MaterialDefinition, 5>& getMaterialCatalog() {
        static const std::array<MaterialDefinition, 5> materials{{
            {Material_Default, "default", 0x8C8264},
            {Material_Grass, "grass", 0x4F7D32},
            {Material_Concrete, "concrete", 0x777568},
            {Material_RustMetal, "rust_metal", 0x8A4C2A},
            {Material_Dirt, "dirt", 0x5B4935},
        }};
        return materials;
    }

    inline const MaterialDefinition& getMaterial(std::uint32_t materialID) {
        const auto& materials = getMaterialCatalog();
        for (const auto& material : materials) {
            if (material.id == materialID) {
                return material;
            }
        }
        return materials[Material_Default];
    }

} // namespace bunker
