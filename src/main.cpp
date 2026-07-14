#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "A.I.M.P. Engine v0.1");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) return -1;

    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // --- ИНТЕРФЕЙС A.I.M.P. ---
        ImGuiWindowFlags hud_flags = ImGuiWindowFlags_NoDecoration | 
                                     ImGuiWindowFlags_AlwaysAutoResize | 
                                     ImGuiWindowFlags_NoSavedSettings | 
                                     ImGuiWindowFlags_NoFocusOnAppearing | 
                                     ImGuiWindowFlags_NoNav | 
                                     ImGuiWindowFlags_NoBackground |
                                     ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::Begin("AIMP_HUD", nullptr, hud_flags);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::Text("A.I.M.P. PROTOCOL INITIALIZED");
        ImGui::Text("DRONE UPLINK: STANDBY...");
        ImGui::Separator();
        ImGui::Text("BUILD BUDGET: [|||||     ] 50%%");
        ImGui::PopStyleColor(); 
        
        ImGui::End();

        window.clear(sf::Color(30, 30, 30));
        ImGui::SFML::Render(window);         
        window.display();                    
    }

    ImGui::SFML::Shutdown();
    return 0;
}