#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

#include "MapEditorConsole.hpp"
#include "MapEditorInput.hpp"
#include "MapEditorPanels.hpp"
#include "MapEditorPlacement.hpp"
#include "MapEditorView.hpp"
#include "PlacementSystem.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode({1400, 900}), "Bunker World Editor");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        return -1;
    }

    MapEditorConsole console;
    PlacementSystem placementSystem;

    bool showConsole = true;
    bool showInstructions = true;
    MapEditorPlacementState placementState;

    sf::Clock deltaClock;
    sf::View mapView(sf::FloatRect({0.f, 0.f}, {1400.f, 900.f}));
    mapView.setCenter({0.f, 0.f});

    MapEditorInputState inputState;

    while (window.isOpen())
    {
        // 1. ОБРАБОТКА СОБЫТИЙ (SFML 3)
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            HandleMapEditorEvent(window, *event, mapView, deltaClock, placementSystem, placementState, inputState);
        }

        HandleMapEditorKeyboardShortcuts(placementSystem, placementState, inputState);

        // 2. ОБНОВЛЕНИЕ IMGUI
        ImGui::SFML::Update(window, deltaClock.restart());

        // 3. ОТРИСОВКА ОКОН

        // --- Window: Map View ---
        inputState.isMouseOverMapView =
            DrawMapViewWindow(inputState.allowWindowMove, mapView, placementState.currentLayer, inputState.hoveredX,
                              inputState.hoveredY, placementSystem);

        // --- Window: Placement ---
        DrawPlacementWindow(inputState.allowWindowMove, placementState);

        // --- Window: Console ---
        DrawConsoleWindow(showConsole, inputState.allowWindowMove, console);

        // --- Window: Instructions ---
        DrawInstructionsWindow(showInstructions);

        // 4. ФИНАЛЬНЫЙ РЕНДЕР
        window.clear(sf::Color(25, 25, 30));
        window.setView(mapView);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
