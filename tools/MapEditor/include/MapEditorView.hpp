#pragma once

#include <SFML/Graphics.hpp>

class PlacementSystem;

constexpr int MAP_WIDTH = 32;
constexpr int MAP_HEIGHT = 32;
constexpr float TILE_WIDTH = 64.f;
constexpr float TILE_HEIGHT = 32.f;

sf::Vector2f isoToScreen(int x, int y, float z = 0.0f);
bool DrawMapViewWindow(bool allowWindowMove, const sf::View& mapView, int currentLayer, int hoveredX, int hoveredY,
                       const PlacementSystem& placementSystem);
