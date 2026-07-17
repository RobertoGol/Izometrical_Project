#include "MapEditorPlacement.hpp"

#include "PlacementSystem.hpp"
#include <imgui.h>
#include <string>
#include <vector>

namespace
{
    constexpr const char* Materials[] = {"concrete",   "metal",       "wood",     "brick",
                                         "reinforced", "rusty_metal", "plasteel", "stone"};
    constexpr const char* ModNames[] = {"clean",       "rusty",        "damaged",  "reinforced",
                                        "painted_red", "painted_blue", "graffiti", "moss"};
    constexpr const char* PropTypes[] = {"crate_wood", "crate_metal", "barrel", "tree_pine", "tree_oak", "stair_metal"};
} // namespace

void HandlePlacementClick(PlacementSystem& placementSystem, MapEditorPlacementState& state, sf::Mouse::Button button,
                          int gridX, int gridY, bool& showContextMenu, int& contextX, int& contextY)
{
    if (button == sf::Mouse::Button::Right)
    {
        contextX = gridX;
        contextY = gridY;
        showContextMenu = true;
        return;
    }

    if (button != sf::Mouse::Button::Left)
    {
        return;
    }

    switch (state.placementType)
    {
    case 0:
    {
        std::vector<std::string> mods;
        if (state.selectedMod > 0)
        {
            mods.push_back(ModNames[state.selectedMod]);
        }
        placementSystem.PlaceWall(gridX, gridY, Materials[state.selectedMaterial], state.selectedHeight + 1,
                                  state.currentLayer, mods);
        break;
    }
    case 1:
        placementSystem.PlaceProp(gridX, gridY, PropTypes[state.selectedProp], state.currentLayer,
                                  state.currentRotation);
        break;
    case 2:
        placementSystem.PlaceTile(gridX, gridY, "concrete", state.currentLayer);
        break;
    default:
        break;
    }
}

void DrawPlacementWindow(bool allowWindowMove, MapEditorPlacementState& state)
{
    ImGuiWindowFlags placementFlags = allowWindowMove ? 0 : ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("Placement", nullptr, placementFlags))
    {
        const char* layers[] = {"Ground", "Objects", "Entities", "Triggers", "Effects"};
        for (int i = 0; i < 5; ++i)
        {
            bool visible = state.layerVisibility[i];
            if (ImGui::Checkbox(layers[i], &visible))
            {
                state.layerVisibility[i] = visible;
            }
            if (ImGui::IsItemClicked())
            {
                state.currentLayer = i;
            }
        }

        ImGui::Separator();
        ImGui::Text("Placement Type");
        ImGui::RadioButton("Wall", &state.placementType, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Prop", &state.placementType, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Tile", &state.placementType, 2);
        ImGui::Separator();

        switch (state.placementType)
        {
        case 0:
            ImGui::Text("Wall Settings");
            ImGui::SliderFloat("Rotation", &state.currentRotation, 0.0f, 360.0f, "%.0fВ°");
            ImGui::Combo("Material", &state.selectedMaterial,
                         "concrete\0metal\0wood\0brick\0reinforced\0rusty_metal\0plasteel\0stone\0");
            ImGui::Combo("Height", &state.selectedHeight, "1m\02m\03m\04m\0");
            ImGui::Combo("Modifier", &state.selectedMod,
                         "clean\0rusty\0damaged\0reinforced\0painted_red\0painted_blue\0graffiti\0moss\0");
            break;
        case 1:
            ImGui::Text("Prop Settings");
            ImGui::SliderFloat("Rotation", &state.currentRotation, 0.0f, 360.0f, "%.0fВ°");
            ImGui::Combo("Prop Type", &state.selectedProp,
                         "crate_wood\0crate_metal\0barrel\0tree_pine\0tree_oak\0stair_metal\0");
            break;
        case 2:
            ImGui::Text("Tile Settings");
            ImGui::TextDisabled("Click on map to place tile");
            break;
        default:
            break;
        }
    }
    ImGui::End();
}
