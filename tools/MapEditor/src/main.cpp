#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <vector>
#include <string>

enum class LayerType
{
    Ground = 0,
    Objects,
    Entities,
    Triggers,
    Effects,
    COUNT
};

const char *layerNames[] = {"Ground", "Objects", "Entities", "Triggers", "Effects"};

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Bunker World Editor");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window))
    {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        return -1;
    }

    bool showConsole = true;
    bool showInstructions = true;
    int currentLayer = 0;

    std::vector<bool> layerVisibility(5, true);
    std::string consoleInput;
    std::vector<std::string> consoleHistory;

    sf::Clock deltaClock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // === Main Menu Bar ===
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Map (.bwld)"))
                {
                }
                if (ImGui::MenuItem("Load Map (.bwld)"))
                {
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Console", nullptr, &showConsole);
                ImGui::MenuItem("Instructions", nullptr, &showInstructions);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // === Map View ===
        ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(900, 650), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Map View"))
        {
            ImGui::Text("Isometric Map Preview (placeholder)");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Здесь будет изометрическая сетка");

            ImGui::Separator();
            ImGui::Text("Layers:");

            for (int i = 0; i < 5; ++i)
            {
                bool visible = layerVisibility[i];
                if (ImGui::Checkbox(layerNames[i], &visible))
                {
                    layerVisibility[i] = visible;
                }
                ImGui::SameLine();
            }
        }
        ImGui::End();

        // === Console ===
        if (showConsole)
        {
            ImGui::SetNextWindowPos(ImVec2(920, 30), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Console"))
            {
                ImGui::InputText("##cmd", &consoleInput, ImGuiInputTextFlags_EnterReturnsTrue);

                if (ImGui::Button("Execute") || (ImGui::IsKeyPressed(ImGuiKey_Enter) && !consoleInput.empty()))
                {
                    if (!consoleInput.empty())
                    {
                        consoleHistory.push_back("> " + consoleInput);
                        consoleInput.clear();
                    }
                }

                ImGui::BeginChild("History", ImVec2(0, 200), true);
                for (const auto &line : consoleHistory)
                {
                    ImGui::TextUnformatted(line.c_str());
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }

        // === Instructions ===
        if (showInstructions)
        {
            ImGui::SetNextWindowPos(ImVec2(920, 350), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Instructions"))
            {
                ImGui::TextWrapped("Вставь сюда промпт для ИИ + изображения локаций.\n\nПример:\n\"Создай небольшую военную базу с 3 зданиями, 2 турелями и спавном игрока.\"");
                ImGui::Separator();
                ImGui::TextDisabled("Drag & Drop изображений сюда (пока не реализовано)");
            }
            ImGui::End();
        }

        window.clear(sf::Color(30, 30, 30));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}