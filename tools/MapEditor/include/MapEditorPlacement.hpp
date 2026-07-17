#pragma once

#include <SFML/Graphics.hpp>

class PlacementSystem;

struct MapEditorPlacementState
{
    int currentLayer = 0;
    bool layerVisibility[5] = {true, true, true, true, true};
    int selectedMaterial = 0;
    int selectedHeight = 0;
    int selectedMod = 0;
    int placementType = 0;
    float currentRotation = 0.0f;
    int selectedProp = 0;
};

void HandlePlacementClick(PlacementSystem& placementSystem, MapEditorPlacementState& state, sf::Mouse::Button button,
                          int gridX, int gridY, bool& showContextMenu, int& contextX, int& contextY);
void DrawPlacementWindow(bool allowWindowMove, MapEditorPlacementState& state);
