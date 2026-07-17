#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <glm/glm.hpp>

namespace bunker {

    enum MaterialId : std::uint32_t {
        Material_Default = 0,
        Material_Grass = 1,
        Material_Concrete = 2,
        Material_RustMetal = 3,
        Material_Dirt = 4
    };

    enum class MaterialCategory : std::uint32_t {
        Terrain,
        Metal,
        Concrete,
        Organic,
        Debug
    };

    struct MaterialTextureSlots {
        const char* albedo = "";
        const char* normal = "";
        const char* roughness = "";
        const char* metallic = "";
    };

    struct MaterialDefinition {
        std::uint32_t id = Material_Default;
        const char* name = "default";
        std::uint32_t hex = 0x8C8264;
        MaterialCategory category = MaterialCategory::Debug;
        float roughness = 0.85f;
        float metallic = 0.0f;
        std::uint32_t emissiveHex = 0x000000;
        MaterialTextureSlots textures;
    };

    inline glm::vec3 hexToLinearRgb(std::uint32_t hex) {
        const float r = static_cast<float>((hex >> 16) & 0xFF) / 255.0f;
        const float g = static_cast<float>((hex >> 8) & 0xFF) / 255.0f;
        const float b = static_cast<float>(hex & 0xFF) / 255.0f;
        return {r, g, b};
    }

    inline const std::array<MaterialDefinition, 5>& getMaterialCatalog() {
        static const std::array<MaterialDefinition, 5> materials{{
            {Material_Default, "default", 0x8C8264, MaterialCategory::Debug, 0.85f, 0.0f, 0x000000, {}},
            {Material_Grass,
             "grass",
             0x4F7D32,
             MaterialCategory::Terrain,
             0.95f,
             0.0f,
             0x000000,
             {"assets/generated/ground/ground_0.png", "assets/generated/ground/ground_0_normal.png", "", ""}},
            {Material_Concrete,
             "concrete",
             0x777568,
             MaterialCategory::Concrete,
             0.88f,
             0.0f,
             0x000000,
             {"assets/generated/walls/wall_0.png", "assets/generated/walls/wall_0_normal.png", "", ""}},
            {Material_RustMetal,
             "rust_metal",
             0x8A4C2A,
             MaterialCategory::Metal,
             0.72f,
             0.65f,
             0x000000,
             {"assets/generated/items/item_0.png", "assets/generated/items/item_0_normal.png", "", ""}},
            {Material_Dirt,
             "dirt",
             0x5B4935,
             MaterialCategory::Terrain,
             0.98f,
             0.0f,
             0x000000,
             {"assets/generated/ground/ground_1.png", "assets/generated/ground/ground_1_normal.png", "", ""}},
        }};
        return materials;
    }

    inline const char* materialCategoryName(MaterialCategory category) {
        switch (category) {
        case MaterialCategory::Terrain:
            return "terrain";
        case MaterialCategory::Metal:
            return "metal";
        case MaterialCategory::Concrete:
            return "concrete";
        case MaterialCategory::Organic:
            return "organic";
        case MaterialCategory::Debug:
            return "debug";
        default:
            return "unknown";
        }
    }

    inline bool validateMaterialCatalog(std::string* error = nullptr) {
        const auto& materials = getMaterialCatalog();
        if (materials.empty() || materials[Material_Default].id != Material_Default) {
            if (error != nullptr) {
                *error = "default material must be at Material_Default";
            }
            return false;
        }

        for (std::size_t i = 0; i < materials.size(); ++i) {
            for (std::size_t j = i + 1; j < materials.size(); ++j) {
                if (materials[i].id == materials[j].id) {
                    if (error != nullptr) {
                        *error = "duplicate material id";
                    }
                    return false;
                }
            }
        }

        return true;
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
