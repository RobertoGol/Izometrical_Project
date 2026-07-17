#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class PlacementSystem;
struct MapEditorPlacementState;

struct MapEditorInputState
{
    bool isMouseOverMapView = false;
    bool allowWindowMove = false;
    int hoveredX = 0;
    int hoveredY = 0;
    bool showContextMenu = false;
    int contextX = 0;
    int contextY = 0;
    bool isDraggingCamera = false;
    sf::Vector2f lastMouseWorldPos;
};

void HandleMapEditorEvent(sf::RenderWindow& window, const sf::Event& event, sf::View& mapView,
                          const sf::Clock& deltaClock, PlacementSystem& placementSystem,
                          MapEditorPlacementState& placementState, MapEditorInputState& inputState);
void HandleMapEditorKeyboardShortcuts(PlacementSystem& placementSystem, const MapEditorPlacementState& placementState,
                                      const MapEditorInputState& inputState);
