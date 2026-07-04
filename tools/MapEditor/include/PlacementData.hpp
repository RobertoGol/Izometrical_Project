#pragma once

#include <string>
#include <vector>
#include <map>

// ==================== МАТЕРИАЛЫ ====================
namespace Materials
{
    inline const std::vector<std::string> WallMaterials = {
        "concrete",    // Бетон
        "metal",       // Металл
        "wood",        // Дерево
        "brick",       // Кирпич
        "reinforced",  // Усиленный бетон
        "rusty_metal", // Ржавый металл
        "plasteel",    // Пласталь
        "stone"        // Камень
    };

    inline const std::vector<std::string> FloorMaterials = {
        "concrete",
        "metal_grid",
        "wood",
        "tile",
        "dirt",
        "gravel",
        "asphalt"};
}

// ==================== МОДИФИКАЦИИ ====================
namespace Modifiers
{
    inline const std::vector<std::string> WallMods = {
        "clean",        // Чистый
        "rusty",        // Ржавый
        "damaged",      // Повреждённый
        "reinforced",   // Усиленный
        "painted_red",  // Красная краска
        "painted_blue", // Синяя краска
        "graffiti",     // Граффити
        "moss",         // Мох
        "blood",        // Кровь
        "burned"        // Обгоревший
    };

    inline const std::vector<std::string> PropMods = {
        "clean",
        "damaged",
        "rusty",
        "overgrown",
        "painted"};
}

// ==================== ТИПЫ СТЕН ====================
namespace WallTypes
{
    inline const std::map<std::string, std::string> WallDisplayNames = {
        {"concrete", "Бетонная стена"},
        {"metal", "Металлическая стена"},
        {"wood", "Деревянная стена"},
        {"brick", "Кирпичная стена"},
        {"reinforced", "Усиленная стена"},
        {"rusty_metal", "Ржавая металлическая стена"},
        {"plasteel", "Пласталь"},
        {"stone", "Каменная стена"}};
}

// ==================== ВЫСОТЫ ====================
namespace Heights
{
    inline const std::vector<int> WallHeights = {1, 2, 3, 4}; // в метрах
}

// ==================== ПРОПСЫ ====================
namespace Props
{
    inline const std::vector<std::string> Trees = {
        "tree_pine",
        "tree_oak",
        "tree_dead",
        "tree_birch"};

    inline const std::vector<std::string> Furniture = {
        "crate_wood",
        "crate_metal",
        "barrel",
        "table",
        "chair",
        "shelf"};

    inline const std::vector<std::string> Stairs = {
        "stair_metal",
        "stair_concrete",
        "stair_wood",
        "ladder_metal"};
}