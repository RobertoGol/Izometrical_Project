#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "PlacementSystem.hpp"
#include "PlacementData.hpp"

const int MAP_WIDTH = 32, MAP_HEIGHT = 32;
const float TILE_WIDTH = 64.f, TILE_HEIGHT = 32.f;
inline bool dragTriggered = false;

sf::Vector2f isoToScreen(int x, int y)
{
    return {(x - y) * (TILE_WIDTH / 2.f), (x + y) * (TILE_HEIGHT / 2.f)};
}

class MapEditorConsole
{
public:
    std::vector<std::string> history;
    std::string input;

    void Execute(const std::string &cmd, PlacementSystem &sys)
    {
        history.push_back(">" + cmd);
        std::istringstream iss(cmd);
        std::string command;
        iss >> command;

        if (command == "clear")
        {
            history.clear();
        }
        else if (command == "save")
        {
            if (sys.SaveToFile("map.dat"))
                history.push_back("[System] Map auto-saved to map.dat");
            else
                history.push_back("[Error] Failed to auto-save map!");
        }
        else
        {
            history.push_back("[Console] Unknown command: " + command);
        }
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Bunker World Editor");
    window.setFramerateLimit(60);

    // Ставим true: ImGui сам железно загрузит свой встроенный шрифт. Никаких ассертов и крашей.
    if (!ImGui::SFML::Init(window, true))
        return -1;

    ImGuiIO &io = ImGui::GetIO();

    MapEditorConsole console;
    PlacementSystem placementSystem;

    bool showConsole = true, showInstructions = true;
    int currentLayer = 0;
    std::vector<bool> layerVisibility(5, true);

    static int placementType = 0;
    static int selectedMat = 0;
    static int selectedHeight = 0;
    static int selectedMod = 0;
    static int selectedPropCat = 0;
    static int selectedProp = 0;
    static int selectedTileMat = 0;
    static float currentRotation = 0.0f;

    int hoveredX = 0, hoveredY = 0, contextX = 0, contextY = 0;
    bool showContextMenu = false;

    sf::Clock deltaClock;
    sf::View mapView(sf::FloatRect({0.f, 0.f}, {1400.f, 900.f}));
    mapView.setCenter({0.f, 0.f});

    bool isPanning = false;
    sf::Vector2i panStartMousePos;
    sf::Vector2f panStartCameraCenter;

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();

            if (io.WantCaptureMouse)
                continue;

            if (const auto *mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                mapView.zoom(1.f - mouseWheel->delta * 0.1f);
            }

            if (const auto *mouseBtn = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mouseBtn->button == sf::Mouse::Button::Left && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F10))
                {
                    isPanning = true;
                    panStartMousePos = sf::Mouse::getPosition(window);
                    panStartCameraCenter = mapView.getCenter();
                    dragTriggered = true;
                }
                else if (mouseBtn->button == sf::Mouse::Button::Left && !isPanning)
                {
                    sf::Vector2i mPos = sf::Mouse::getPosition(window);
                    sf::Vector2f wPos = window.mapPixelToCoords(mPos, mapView);

                    float isoX = (wPos.x / (TILE_WIDTH / 2.f) + wPos.y / (TILE_HEIGHT / 2.f)) / 2.f;
                    float isoY = (wPos.y / (TILE_HEIGHT / 2.f) - wPos.x / (TILE_WIDTH / 2.f)) / 2.f;

                    hoveredX = static_cast<int>(std::round(isoX));
                    hoveredY = static_cast<int>(std::round(isoY));

                    placementSystem.SnapToGrid(hoveredX, hoveredY);

                    if (placementType == 0)
                    {
                        std::string mat = Materials::WallMaterials[selectedMat];
                        int height = Heights::WallHeights[selectedHeight];
                        std::vector<std::string> mods = {Modifiers::WallMods[selectedMod]};
                        placementSystem.PlaceWall(hoveredX, hoveredY, mat, height, currentLayer, mods);
                    }
                    else if (placementType == 1)
                    {
                        std::string propId = "";
                        if (selectedPropCat == 0)
                            propId = Props::Trees[selectedProp];
                        else if (selectedPropCat == 1)
                            propId = Props::Furniture[selectedProp];
                        else if (selectedPropCat == 2)
                            propId = Props::Stairs[selectedProp];

                        if (!propId.empty())
                        {
                            placementSystem.PlaceProp(hoveredX, hoveredY, propId, currentLayer, currentRotation);
                        }
                    }
                    else if (placementType == 2)
                    {
                        std::string tileMat = Materials::FloorMaterials[selectedTileMat];
                        placementSystem.PlaceTile(hoveredX, hoveredY, tileMat, currentLayer);
                    }
                }
                else if (mouseBtn->button == sf::Mouse::Button::Right)
                {
                    contextX = hoveredX;
                    contextY = hoveredY;
                    showContextMenu = true;
                }
            }

            if (const auto *mouseBtnRelease = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (mouseBtnRelease->button == sf::Mouse::Button::Left)
                    isPanning = false;
            }

            if (const auto *mouseMove = event->getIf<sf::Event::MouseMoved>())
            {
                if (isPanning)
                {
                    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
                    sf::Vector2i mDelta = currentMousePos - panStartMousePos;
                    sf::Vector2f wZero = window.mapPixelToCoords({0, 0}, mapView);
                    sf::Vector2f wDelta = window.mapPixelToCoords(mDelta, mapView);
                    mapView.setCenter(panStartCameraCenter + (wZero - wDelta));
                }
                else
                {
                    sf::Vector2i mPos = sf::Mouse::getPosition(window);
                    sf::Vector2f wPos = window.mapPixelToCoords(mPos, mapView);

                    float isoX = (wPos.x / (TILE_WIDTH / 2.f) + wPos.y / (TILE_HEIGHT / 2.f)) / 2.f;
                    float isoY = (wPos.y / (TILE_HEIGHT / 2.f) - wPos.x / (TILE_WIDTH / 2.f)) / 2.f;

                    hoveredX = static_cast<int>(std::round(isoX));
                    hoveredY = static_cast<int>(std::round(isoY));
                    placementSystem.SnapToGrid(hoveredX, hoveredY);
                }
            }
        }

        if (!io.WantCaptureMouse && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Delete) && (hoveredX != 0 || hoveredY != 0))
        {
            placementSystem.RemoveObject(hoveredX, hoveredY, currentLayer);
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            dragTriggered = false;

        // === ВЕРХНЕЕ МЕНЮ ===
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Map"))
                {
                    if (placementSystem.SaveToFile("map.dat"))
                        console.history.push_back("[System] Map saved successfully.");
                    else
                        console.history.push_back("[Error] Failed to save map.");
                }
                if (ImGui::MenuItem("Load Map"))
                {
                    if (placementSystem.LoadFromFile("map.dat"))
                        console.history.push_back("[System] Map loaded successfully.");
                    else
                        console.history.push_back("[Error] Map file not found.");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // === ОКНО: MAP VIEW ===
        ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(950, 800), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Map View"))
        {
            ImGui::Text("Isometric Map Editor - Active Layer: %d", currentLayer);

            if (ImGui::BeginChild("MapCanvas", ImVec2(900, 700), true))
            {
                ImDrawList *dL = ImGui::GetWindowDrawList();
                ImVec2 cPos = ImGui::GetCursorScreenPos();

                // 1. Отрисовка сетки
                for (int y = 0; y < MAP_HEIGHT; ++y)
                {
                    for (int x = 0; x < MAP_WIDTH; ++x)
                    {
                        sf::Vector2f p = isoToScreen(x, y);
                        p.x += cPos.x + 400;
                        p.y += cPos.y + 100;

                        ImVec2 pts[] = {
                            ImVec2(p.x, p.y),
                            ImVec2(p.x + TILE_WIDTH / 2.f, p.y + TILE_HEIGHT / 2.f),
                            ImVec2(p.x, p.y + TILE_HEIGHT),
                            ImVec2(p.x - TILE_WIDTH / 2.f, p.y + TILE_HEIGHT / 2.f)};

                        if (x == hoveredX && y == hoveredY)
                        {
                            dL->AddConvexPolyFilled(pts, 4, ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 0.4f, 0.5f)));
                        }
                        dL->AddPolyline(pts, 4, ImGui::ColorConvertFloat4ToU32(ImVec4(0.25f, 0.25f, 0.25f, 0.4f)), ImDrawFlags_Closed, 1.f);
                    }
                }

                // 2. Изометрическая сортировка (Глубина и Слои)
                std::vector<PlacedObject> visibleObjects;
                for (const auto &obj : placementSystem.GetAllObjects())
                {
                    if (obj.layer >= 0 && obj.layer < 5 && layerVisibility[obj.layer])
                    {
                        visibleObjects.push_back(obj);
                    }
                }

                std::sort(visibleObjects.begin(), visibleObjects.end(), [](const PlacedObject &a, const PlacedObject &b)
                          {
                    if (a.layer != b.layer) {
                        return a.layer < b.layer; 
                    }
                    if (a.category != b.category) {
                        return a.category == PlacementCategory::Tile; 
                    }
                    if ((a.x + a.y) != (b.x + b.y)) {
                        return (a.x + a.y) < (b.x + b.y); 
                    }
                    return a.x < b.x; });

                // 3. Рендеринг отсортированных объектов
                for (const auto &obj : visibleObjects)
                {
                    sf::Vector2f p = isoToScreen(obj.x, obj.y);
                    p.x += cPos.x + 400;
                    p.y += cPos.y + 100;

                    sf::Color c;
                    if (obj.category == PlacementCategory::Tile)
                        c = sf::Color(100, 110, 100);
                    else if (obj.category == PlacementCategory::Wall)
                        c = (obj.connectionMask > 0 ? sf::Color(70, 130, 180) : sf::Color(110, 110, 120));
                    else
                        c = sf::Color(150, 100, 60);

                    ImVec2 pts[] = {
                        ImVec2(p.x, p.y),
                        ImVec2(p.x + TILE_WIDTH / 2.f, p.y + TILE_HEIGHT / 2.f),
                        ImVec2(p.x, p.y + TILE_HEIGHT),
                        ImVec2(p.x - TILE_WIDTH / 2.f, p.y + TILE_HEIGHT / 2.f)};

                    dL->AddConvexPolyFilled(pts, 4, ImGui::ColorConvertFloat4ToU32(ImVec4(c.r / 255.f, c.g / 255.f, c.b / 255.f, 1.f)));
                    dL->AddPolyline(pts, 4, ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.f, 0.f, 1.f)), ImDrawFlags_Closed, 1.5f);
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();

        // === ОКНО: ИНСТРУМЕНТЫ РАЗМЕЩЕНИЯ ===
        ImGui::SetNextWindowPos(ImVec2(980, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 360), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Placement"))
        {
            ImGui::Text("Active Layer Select:");
            const char *layerNames[] = {"Ground (0)", "Objects (1)", "Entities (2)", "Triggers (3)", "Effects (4)"};
            ImGui::Combo("Active Layer", &currentLayer, layerNames, 5);

            ImGui::Separator();
            ImGui::Text("Layers Visibility:");
            for (int i = 0; i < 5; ++i)
            {
                bool vis = layerVisibility[i];
                if (ImGui::Checkbox(layerNames[i], &vis))
                    layerVisibility[i] = vis;
            }

            ImGui::Separator();
            ImGui::Text("Placement Type");
            ImGui::RadioButton("Wall", &placementType, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Prop", &placementType, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Tile", &placementType, 2);
            ImGui::Separator();

            if (placementType == 0)
            {
                std::string currentMatKey = Materials::WallMaterials[selectedMat];
                std::string currentMatLabel = currentMatKey;
                if (WallTypes::WallDisplayNames.count(currentMatKey))
                {
                    currentMatLabel = WallTypes::WallDisplayNames.at(currentMatKey);
                }

                if (ImGui::BeginCombo("Material", currentMatLabel.c_str()))
                {
                    for (size_t i = 0; i < Materials::WallMaterials.size(); ++i)
                    {
                        bool isSelected = (selectedMat == static_cast<int>(i));
                        std::string key = Materials::WallMaterials[i];
                        std::string label = WallTypes::WallDisplayNames.count(key) ? WallTypes::WallDisplayNames.at(key) : key;

                        if (ImGui::Selectable(label.c_str(), isSelected))
                            selectedMat = static_cast<int>(i);
                    }
                    ImGui::EndCombo();
                }

                std::string currentHeightStr = std::to_string(Heights::WallHeights[selectedHeight]) + "m";
                if (ImGui::BeginCombo("Height", currentHeightStr.c_str()))
                {
                    for (size_t i = 0; i < Heights::WallHeights.size(); ++i)
                    {
                        bool isSelected = (selectedHeight == static_cast<int>(i));
                        std::string hStr = std::to_string(Heights::WallHeights[i]) + "m";
                        if (ImGui::Selectable(hStr.c_str(), isSelected))
                            selectedHeight = static_cast<int>(i);
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::BeginCombo("Modifier", Modifiers::WallMods[selectedMod].c_str()))
                {
                    for (size_t i = 0; i < Modifiers::WallMods.size(); ++i)
                    {
                        bool isSelected = (selectedMod == static_cast<int>(i));
                        if (ImGui::Selectable(Modifiers::WallMods[i].c_str(), isSelected))
                            selectedMod = static_cast<int>(i);
                    }
                    ImGui::EndCombo();
                }
            }
            else if (placementType == 1)
            {
                const char *propCategories[] = {"Trees", "Furniture", "Stairs"};
                if (ImGui::BeginCombo("Prop Category", propCategories[selectedPropCat]))
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        if (ImGui::Selectable(propCategories[i], selectedPropCat == i))
                        {
                            selectedPropCat = i;
                            selectedProp = 0;
                        }
                    }
                    ImGui::EndCombo();
                }

                const std::vector<std::string> *activePropVector = nullptr;
                if (selectedPropCat == 0)
                    activePropVector = &Props::Trees;
                else if (selectedPropCat == 1)
                    activePropVector = &Props::Furniture;
                else if (selectedPropCat == 2)
                    activePropVector = &Props::Stairs;

                if (activePropVector && !activePropVector->empty())
                {
                    if (selectedProp >= static_cast<int>(activePropVector->size()))
                        selectedProp = 0;

                    if (ImGui::BeginCombo("Object ID", (*activePropVector)[selectedProp].c_str()))
                    {
                        for (size_t i = 0; i < activePropVector->size(); ++i)
                        {
                            if (ImGui::Selectable((*activePropVector)[i].c_str(), selectedProp == static_cast<int>(i)))
                                selectedProp = static_cast<int>(i);
                        }
                        ImGui::EndCombo();
                    }
                }
                ImGui::SliderFloat("Rotation", &currentRotation, 0.0f, 360.0f, "%.1f deg");
            }
            else if (placementType == 2)
            {
                if (ImGui::BeginCombo("Floor Material", Materials::FloorMaterials[selectedTileMat].c_str()))
                {
                    for (size_t i = 0; i < Materials::FloorMaterials.size(); ++i)
                    {
                        if (ImGui::Selectable(Materials::FloorMaterials[i].c_str(), selectedTileMat == static_cast<int>(i)))
                            selectedTileMat = static_cast<int>(i);
                    }
                    ImGui::EndCombo();
                }
            }
        }
        ImGui::End();

        // === ОКНО: КОНСОЛЬ ===
        if (showConsole)
        {
            ImGui::SetNextWindowPos(ImVec2(980, 410), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 270), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Map Editor Console"))
            {
                if (ImGui::InputText("##cmd", &console.input, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (!console.input.empty())
                    {
                        console.Execute(console.input, placementSystem);
                        console.input.clear();
                    }
                }

                if (ImGui::BeginChild("History", ImVec2(0, 180), true))
                {
                    for (const auto &line : console.history)
                    {
                        ImGui::TextUnformatted(line.c_str());
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }

        if (showInstructions)
        {
            ImGui::SetNextWindowPos(ImVec2(980, 700), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 170), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Instructions"))
            {
                ImGui::TextWrapped("Controls:\nF10 + LMB: Pan Map\nLMB: Place Object\nRMB: Context Menu\nDelete: Remove Object");
                ImGui::End();
            }
        }

        if (showContextMenu)
        {
            ImGui::OpenPopup("ContextMenu");
            showContextMenu = false;
        }

        if (ImGui::BeginPopup("ContextMenu"))
        {
            ImGui::Text("Object (%d, %d)", contextX, contextY);
            ImGui::Separator();
            if (ImGui::MenuItem("Delete on Active Layer"))
                placementSystem.RemoveObject(contextX, contextY, currentLayer);
            if (ImGui::MenuItem("Delete All at Cell"))
                placementSystem.RemoveObject(contextX, contextY, -1);
            ImGui::EndPopup();
        }

        window.clear(sf::Color(25, 25, 30));
        window.setView(mapView);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}