#pragma once

#include "WorkstationTypes.hpp"
#include <vector>
#include <unordered_map>

class RecipeDatabase
{
public:
    static RecipeDatabase &Get();

    void LoadDefaultRecipes();

    std::vector<CraftingRecipe> GetRecipesForStation(WorkstationType type) const;
    const CraftingRecipe *GetRecipeById(int id) const;

private:
    RecipeDatabase() = default;
    std::vector<CraftingRecipe> allRecipes;
    std::unordered_map<WorkstationType, std::vector<int>> stationRecipes; // recipe IDs per station
};