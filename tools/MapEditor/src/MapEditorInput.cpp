#include "MapEditorInput.hpp"

#include "MapEditorPlacement.hpp"
#include "MapEditorView.hpp"
#include "PlacementSystem.hpp"
#include <cmath>
#include <imgui.h>

namespace
{
    void HandleCameraInput(sf::RenderWindow& window, const sf::Event& event, sf::View& mapView,
                           const sf::Clock& deltaClock, MapEditorInputState& inputState)
    {
        if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            mapView.zoom(1.f - wheel->delta * 0.1f);
        }

        if (!inputState.isMouseOverMapView || !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            return;
        }

        float moveSpeed = 500.0f * deltaClock.getElapsedTime().asSeconds();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            mapView.move({0.f, -moveSpeed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            mapView.move({0.f, moveSpeed});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            mapView.move({-moveSpeed, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            mapView.move({moveSpeed, 0.f});

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add))
            mapView.zoom(0.95f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract))
            mapView.zoom(1.05f);

        if (event.is<sf::Event::MouseButtonPressed>() &&
            event.getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Middle)
        {
            inputState.isDraggingCamera = true;
            inputState.lastMouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), mapView);
        }
        if (event.is<sf::Event::MouseButtonReleased>() &&
            event.getIf<sf::Event::MouseButtonReleased>()->button == sf::Mouse::Button::Middle)
        {
            inputState.isDraggingCamera = false;
        }
    }

    void HandleMapClick(sf::RenderWindow& window, const sf::Event& event, sf::View& mapView,
                        PlacementSystem& placementSystem, MapEditorPlacementState& placementState,
                        MapEditorInputState& inputState)
    {
        if (!event.is<sf::Event::MouseButtonPressed>())
        {
            return;
        }

        const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, mapView);

        float isoX = (worldPos.x / (TILE_WIDTH / 2.f) + worldPos.y / (TILE_HEIGHT / 2.f)) / 2.0f;
        float isoY = (worldPos.y / (TILE_HEIGHT / 2.f) - worldPos.x / (TILE_WIDTH / 2.f)) / 2.0f;

        int gridX = static_cast<int>(std::round(isoX));
        int gridY = static_cast<int>(std::round(isoY));

        placementSystem.SnapToGrid(gridX, gridY);
        inputState.hoveredX = gridX;
        inputState.hoveredY = gridY;

        HandlePlacementClick(placementSystem, placementState, mouse->button, gridX, gridY, inputState.showContextMenu,
                             inputState.contextX, inputState.contextY);
    }
} // namespace

void HandleMapEditorEvent(sf::RenderWindow& window, const sf::Event& event, sf::View& mapView,
                          const sf::Clock& deltaClock, PlacementSystem& placementSystem,
                          MapEditorPlacementState& placementState, MapEditorInputState& inputState)
{
    HandleCameraInput(window, event, mapView, deltaClock, inputState);
    HandleMapClick(window, event, mapView, placementSystem, placementState, inputState);
}

void HandleMapEditorKeyboardShortcuts(PlacementSystem& placementSystem, const MapEditorPlacementState& placementState,
                                      const MapEditorInputState& inputState)
{
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Delete))
    {
        return;
    }

    if (inputState.hoveredX != 0 || inputState.hoveredY != 0)
    {
        placementSystem.RemoveObject(inputState.hoveredX, inputState.hoveredY, placementState.currentLayer);
    }
}
