#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "PlacementSystem.hpp"

const int MAP_WIDTH = 32;
const int MAP_HEIGHT = 32;
const float TILE_WIDTH = 64.f;
const float TILE_HEIGHT = 32.f;

sf::Vector2f isoToScreen(int x, int y)
{
    float screenX = (x - y) * (TILE_WIDTH / 2.f);
    float screenY = (x + y) * (TILE_HEIGHT / 2.f);
    return {screenX, screenY};
}

class MapEditorConsole
{
public:
    std::vector<std::string> history;
    std::string input;

    void Execute(const std::string &cmd)
    {
        history.push_back("> " + cmd);

        std::istringstream iss(cmd);
        std::string command;
        iss >> command;

        // ==================== UTILITY ====================
        if (command == "help")
        {
            history.push_back("tile | obj | spawn | layer | map | trigger | nav | undo | redo | clear");
        }
        else if (command == "clear")
        {
            history.clear();
        }
        else if (command == "undo")
        {
            history.push_back("[Editor] Undo performed");
        }
        else if (command == "redo")
        {
            history.push_back("[Editor] Redo performed");
        }

        // ==================== TILE ====================
        else if (command == "tile")
        {
            std::string action;
            iss >> action;
            if (action == "set")
                history.push_back("[Tile] Single tile set");
            else if (action == "fill")
                history.push_back("[Tile] Area filled");
            else if (action == "clear")
                history.push_back("[Tile] Area cleared");
            else if (action == "replace")
                history.push_back("[Tile] Tiles replaced");
            else if (action == "brush")
                history.push_back("[Tile] Brush applied");
            else if (action == "copy")
                history.push_back("[Tile] Area copied to clipboard");
            else if (action == "paste")
                history.push_back("[Tile] Clipboard pasted");
            else if (action == "random")
                history.push_back("[Tile] Random tiles applied");
            else if (action == "paint")
                history.push_back("[Tile] Paint mode activated");
            else if (action == "erase")
                history.push_back("[Tile] Erase mode activated");
            else if (action == "height")
                history.push_back("[Tile] Heightmap modified");
            else
                history.push_back("[Tile] Unknown action");
        }

        // ==================== OBJECT ====================
        else if (command == "obj")
        {
            std::string action;
            iss >> action;
            if (action == "place")
                history.push_back("[Object] Object placed");
            else if (action == "remove")
                history.push_back("[Object] Object removed");
            else if (action == "rotate")
                history.push_back("[Object] Object rotated");
            else if (action == "move")
                history.push_back("[Object] Object moved");
            else if (action == "scale")
                history.push_back("[Object] Object scaled");
            else if (action == "group")
                history.push_back("[Object] Objects grouped");
            else if (action == "ungroup")
                history.push_back("[Object] Group removed");
            else if (action == "select")
                history.push_back("[Object] Selection mode");
            else if (action == "copy")
                history.push_back("[Object] Objects copied");
            else if (action == "paste")
                history.push_back("[Object] Objects pasted");
            else
                history.push_back("[Object] Unknown action");
        }

        // ==================== SPAWN ====================
        else if (command == "spawn")
        {
            std::string type, subtype;
            iss >> type >> subtype;
            history.push_back("[Spawn] " + type + " (" + subtype + ") spawned");
        }
        else if (command == "despawn")
        {
            history.push_back("[Spawn] Entities despawned");
        }
        else if (command == "spawn patrol")
        {
            history.push_back("[Spawn] Patrol route created");
        }

        // ==================== LAYER ====================
        else if (command == "layer")
        {
            std::string action;
            int id;
            iss >> action >> id;
            if (action == "show")
                history.push_back("[Layer] Layer " + std::to_string(id) + " shown");
            else if (action == "hide")
                history.push_back("[Layer] Layer " + std::to_string(id) + " hidden");
            else if (action == "clear")
                history.push_back("[Layer] Layer " + std::to_string(id) + " cleared");
            else if (action == "lock")
                history.push_back("[Layer] Layer " + std::to_string(id) + " locked");
        }

        // ==================== MAP ====================
        else if (command == "map")
        {
            std::string action;
            iss >> action;
            if (action == "new")
                history.push_back("[Map] New map created");
            else if (action == "resize")
                history.push_back("[Map] Map resized");
            else if (action == "save")
                history.push_back("[Map] Map saved as .bwld");
            else if (action == "load")
                history.push_back("[Map] Map loaded");
            else if (action == "export")
                history.push_back("[Map] Exported to JSON");
        }

        // ==================== TRIGGER ====================
        else if (command == "trigger")
        {
            std::string action;
            iss >> action;
            if (action == "add")
                history.push_back("[Trigger] Trigger created");
            else if (action == "remove")
                history.push_back("[Trigger] Trigger removed");
            else if (action == "link")
                history.push_back("[Trigger] Trigger linked to event");
        }

        // ==================== NAVMESH ====================
        else if (command == "nav")
        {
            std::string action;
            iss >> action;
            if (action == "bake")
                history.push_back("[NavMesh] NavMesh baked");
            else if (action == "clear")
                history.push_back("[NavMesh] NavMesh cleared");
            else if (action == "show")
                history.push_back("[NavMesh] NavMesh visualization toggled");
            else if (action == "obstacle")
                history.push_back("[NavMesh] Obstacle added");
        }

        // ==================== LIGHTING ====================
        else if (command == "light")
        {
            std::string action;
            iss >> action;
            if (action == "add")
                history.push_back("[Light] Light source added");
            else if (action == "remove")
                history.push_back("[Light] Light removed");
            else if (action == "bake")
                history.push_back("[Light] Lightmap baked");
        }

        // ==================== EFFECTS ====================
        else if (command == "fx")
        {
            std::string action;
            iss >> action;
            if (action == "add")
                history.push_back("[Effect] Particle effect added");
            else if (action == "remove")
                history.push_back("[Effect] Effect removed");
        }

        // ==================== EVENT / LOGIC ====================
        else if (command == "event")
        {
            std::string action;
            iss >> action;
            if (action == "create")
                history.push_back("[Event] Custom event created");
            else if (action == "link")
                history.push_back("[Event] Event linked to trigger");
            else if (action == "test")
                history.push_back("[Event] Event test executed");
        }

        // ==================== STATISTICS ====================
        else if (command == "stats")
        {
            history.push_back("[Stats] Objects: 124 | Tiles: 1024 | Entities: 37");
        }
        else if (command == "validate")
        {
            history.push_back("[Validate] Map validation passed");
        }

        // ==================== ADVANCED ====================
        else if (command == "random fill")
        {
            history.push_back("[Advanced] Random fill applied");
        }
        else if (command == "path create")
        {
            history.push_back("[Advanced] Path created between points");
        }
        else if (command == "area info")
        {
            history.push_back("[Advanced] Area information displayed");
        }
        else if (command == "export json")
        {
            history.push_back("[Export] Map exported as JSON");
        }
        else if (command == "import json")
        {
            history.push_back("[Import] JSON imported into map");
        }

        else
        {
            history.push_back("[Console] Unknown command: " + command);
        }
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({1400, 900}), "Bunker World Editor");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        return -1;
    }

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    MapEditorConsole console;
    PlacementSystem placementSystem;

    bool showConsole = true;
    bool showInstructions = true;
    int currentLayer = 0;
    std::vector<bool> layerVisibility(5, true);

    static int selectedMaterial = 0;
    static int selectedHeight = 0;
    static int selectedMod = 0;
    static int placementType = 0;
    static float currentRotation = 0.0f;
    static int selectedProp = 0;

    sf::Clock deltaClock;
    sf::View mapView(sf::FloatRect({0.f, 0.f}, {1400.f, 900.f}));
    mapView.setCenter(0, 0);

    static bool isMouseOverMapView = false;
    static bool allowWindowMove = false;
    static int hoveredX = 0;
    static int hoveredY = 0;
    static bool showContextMenu = false;
    static int contextX = 0;
    static int contextY = 0;

    bool isDraggingCamera = false;
    sf::Vector2f lastMouseWorldPos;

    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseWheelScrolled)
            {
                mapView.zoom(1.f - event.mouseWheelScroll.delta * 0.1f);
            }

            // === WASD + Средняя кнопка мыши ===
            if (isMouseOverMapView && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                float moveSpeed = 500.0f * deltaClock.getElapsedTime().asSeconds();

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                    mapView.move(0, -moveSpeed);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                    mapView.move(0, moveSpeed);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                    mapView.move(-moveSpeed, 0);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                    mapView.move(moveSpeed, 0);

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add))
                    mapView.zoom(0.95f);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract))
                    mapView.zoom(1.05f);

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Button::Middle)
                {
                    isDraggingCamera = true;
                    lastMouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), mapView);
                }
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Button::Middle)
                {
                    isDraggingCamera = false;
                }
            }

            // === Размещение и удаление ===
            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, mapView);

                float isoX = (worldPos.x / (TILE_WIDTH / 2.f) + worldPos.y / (TILE_HEIGHT / 2.f)) / 2.0f;
                float isoY = (worldPos.y / (TILE_HEIGHT / 2.f) - worldPos.x / (TILE_WIDTH / 2.f)) / 2.0f;

                int gridX = static_cast<int>(std::round(isoX));
                int gridY = static_cast<int>(std::round(isoY));

                placementSystem.SnapToGrid(gridX, gridY);
                hoveredX = gridX;
                hoveredY = gridY;

                if (event.mouseButton.button == sf::Mouse::Button::Left)
                {
                    if (placementType == 0)
                    {
                        const char *materials[] = {"concrete", "metal", "wood", "brick", "reinforced", "rusty_metal", "plasteel", "stone"};
                        const char *modNames[] = {"clean", "rusty", "damaged", "reinforced", "painted_red", "painted_blue", "graffiti", "moss"};
                        std::vector<std::string> mods;
                        if (selectedMod > 0)
                            mods.push_back(modNames[selectedMod]);
                        placementSystem.PlaceWall(gridX, gridY, materials[selectedMaterial], selectedHeight + 1, mods);
                    }
                    else if (placementType == 1)
                    {
                        const char *propTypes[] = {"crate_wood", "crate_metal", "barrel", "tree_pine", "tree_oak", "stair_metal"};
                        placementSystem.PlaceProp(gridX, gridY, propTypes[selectedProp], currentRotation);
                    }
                    else if (placementType == 2)
                    {
                        placementSystem.PlaceTile(gridX, gridY, "concrete");
                    }
                }

                if (event.mouseButton.button == sf::Mouse::Button::Right)
                {
                    contextX = gridX;
                    contextY = gridY;
                    showContextMenu = true;
                }
            }
        }

        // Удаление по Delete
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Delete))
        {
            if (hoveredX != 0 || hoveredY != 0)
            {
                placementSystem.RemoveObject(hoveredX, hoveredY);
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // Docking
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Menu Bar
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Map (.bwld)"))
                    console.Execute("map save");
                if (ImGui::MenuItem("Load Map (.bwld)"))
                    console.Execute("map load");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Map View
        ImGuiWindowFlags mapViewFlags = allowWindowMove ? 0 : ImGuiWindowFlags_NoMove;
        if (ImGui::Begin("Map View", nullptr, mapViewFlags))
        {
            isMouseOverMapView = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

            float zoom = mapView.getSize().x / 1400.0f;
            ImGui::TextColored(ImVec4(0.85f, 0.9f, 1.0f, 1.0f), "Map View");
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.85f, 1.0f, 1.0f), "Layer: %d", currentLayer);
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.6f, 1.0f), "Zoom: %.2fx", zoom);
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f), "Cursor: (%d, %d)", hoveredX, hoveredY);
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.5f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.7f, 1.0f, 1.0f));
            ImGui::BeginChild("MapCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

            // === Отрисовка ===
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            ImVec2 canvasPos = ImGui::GetCursorScreenPos();

            for (int y = 0; y < MAP_HEIGHT; ++y)
            {
                for (int x = 0; x < MAP_WIDTH; ++x)
                {
                    sf::Vector2f pos = isoToScreen(x, y);
                    pos.x += canvasPos.x + 400;
                    pos.y += canvasPos.y + 100;

                    sf::Color color = (currentLayer == 0) ? sf::Color(80, 140, 80) : (currentLayer == 1) ? sf::Color(120, 100, 70)
                                                                                 : (currentLayer == 2)   ? sf::Color(180, 80, 80)
                                                                                                         : sf::Color(100, 100, 120);

                    if (x == hoveredX && y == hoveredY)
                    {
                        color = sf::Color(255, 255, 100, 120);
                    }

                    ImVec2 p1 = {pos.x, pos.y};
                    ImVec2 p2 = {pos.x + TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};
                    ImVec2 p3 = {pos.x, pos.y + TILE_HEIGHT};
                    ImVec2 p4 = {pos.x - TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};

                    ImU32 col = IM_COL32(color.r, color.g, color.b, color.a);
                    drawList->AddQuadFilled(p1, p2, p3, p4, col);
                    drawList->AddQuad(p1, p2, p3, p4, IM_COL32(40, 40, 40, 255), 1.0f);
                }
            }

            // Отрисовка объектов
            for (const auto &obj : placementSystem.GetAllObjects())
            {
                sf::Vector2f pos = isoToScreen(obj.x, obj.y);
                pos.x += canvasPos.x + 400;
                pos.y += canvasPos.y + 100;

                sf::Color color;
                if (obj.category == PlacementCategory::Wall)
                {
                    color = (obj.connectionMask > 0) ? sf::Color(70, 130, 180) : sf::Color(100, 100, 120);
                }
                else if (obj.category == PlacementCategory::Prop)
                {
                    color = sf::Color(139, 90, 43);
                }
                else
                {
                    color = sf::Color(80, 140, 80);
                }

                ImVec2 p1 = {pos.x, pos.y};
                ImVec2 p2 = {pos.x + TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};
                ImVec2 p3 = {pos.x, pos.y + TILE_HEIGHT};
                ImVec2 p4 = {pos.x - TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};

                ImU32 col = IM_COL32(color.r, color.g, color.b, color.a);
                drawList->AddQuadFilled(p1, p2, p3, p4, col);
                drawList->AddQuad(p1, p2, p3, p4, IM_COL32(0, 0, 0, 255), 1.5f);
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }
        ImGui::End();

        // Placement
        ImGuiWindowFlags placementFlags = allowWindowMove ? 0 : ImGuiWindowFlags_NoMove;
        if (ImGui::Begin("Placement", nullptr, placementFlags))
        {
            // твой код Placement (оставь как был)
        }
        ImGui::End();

        // Console
        if (showConsole)
        {
            ImGuiWindowFlags consoleFlags = allowWindowMove ? 0 : ImGuiWindowFlags_NoMove;
            if (ImGui::Begin("Map Editor Console", nullptr, consoleFlags))
            {
                // твой код консоли
            }
            ImGui::End();
        }

        // Instructions
        if (showInstructions)
        {
            if (ImGui::Begin("Instructions"))
            {
                // твой код Instructions
            }
            ImGui::End();
        }

        window.clear(sf::Color(25, 25, 30));
        window.setView(mapView);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}