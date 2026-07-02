#include "RecipeDatabase.hpp"

RecipeDatabase &RecipeDatabase::Get()
{
    static RecipeDatabase instance;
    return instance;
}

void RecipeDatabase::LoadDefaultRecipes()
{
    allRecipes.clear();
    stationRecipes.clear();

    // Armor Workbench
    allRecipes.push_back({1, "Reinforced Plating", 25, 3, 1, WorkstationType::ArmorWorkbench});
    allRecipes.push_back({2, "Exosuit Frame", 40, 5, 2, WorkstationType::ArmorWorkbench});

    stationRecipes[WorkstationType::ArmorWorkbench] = {1, 2};

    // Weapons Workbench
    allRecipes.push_back({10, "Assault Rifle", 60, 8, 3, WorkstationType::WeaponsWorkbench});
    stationRecipes[WorkstationType::WeaponsWorkbench] = {10};

    // Add more stations as needed...
}

std::vector<CraftingRecipe> RecipeDatabase::GetRecipesForStation(WorkstationType type) const
{
    std::vector<CraftingRecipe> result;
    auto it = stationRecipes.find(type);
    if (it != stationRecipes.end())
    {
        for (int id : it->second)
        {
            if (const CraftingRecipe *r = GetRecipeById(id))
            {
                result.push_back(*r);
            }
        }
    }
    return result;
}

const CraftingRecipe *RecipeDatabase::GetRecipeById(int id) const
{
    for (const auto &r : allRecipes)
    {
        if (r.id == id)
            return &r;
    }
    return nullptr;
}