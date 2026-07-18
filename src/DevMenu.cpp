#include "DevMenu.hpp"

#ifdef DEV_BUILD

#include "engine/Log.hpp"
#include <imgui.h>
#include <string>
#include <vector>

DevMenu& DevMenu::Get()
{
    static DevMenu instance;
    return instance;
}

void DevMenu::Initialize()
{
    bunker::logInfo() << "[DevMenu] DevMenu initialized (DEV_BUILD)" << std::endl;
    LoadScripts();

    for (int i = 0; i < 5; ++i)
    {
        m_LayerVisibility[i] = true;
    }
}

void DevMenu::Shutdown() {}

void DevMenu::Toggle()
{
    visible = !visible;
}

void DevMenu::Update() {}

void DevMenu::Render()
{
    if (!visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(1200, 750), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Dev Menu", &visible, ImGuiWindowFlags_MenuBar))
    {

        if (ImGui::BeginTabBar("DevTabs"))
        {

            if (ImGui::BeginTabItem("Console"))
            {
                RenderConsoleTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("World"))
            {
                RenderWorldTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Spawn"))
            {
                RenderSpawnTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Player"))
            {
                RenderPlayerTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Map Tools"))
            {
                RenderMapToolsTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug"))
            {
                RenderDebugTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

// ==================== WORLD ====================
static float timeScale = 1.0f;
static float timeOfDay = 12.0f;
static int weatherIndex = 0;
static bool globalLighting = true;

void DevMenu::RenderWorldTab()
{
    ImGui::Text("Time Management");
    ImGui::Separator();

    if (ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 10.0f))
    {
        m_ConsoleHistory.push_back("[World] Time Scale: " + std::to_string(timeScale));
    }

    if (ImGui::Button("Freeze Time"))
    {
        timeScale = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Resume Time"))
    {
        timeScale = 1.0f;
    }

    ImGui::Separator();
    if (ImGui::SliderFloat("Time of Day", &timeOfDay, 0.0f, 24.0f))
    {
    }

    if (ImGui::Button("Morning"))
        timeOfDay = 6.0f;
    ImGui::SameLine();
    if (ImGui::Button("Day"))
        timeOfDay = 12.0f;
    ImGui::SameLine();
    if (ImGui::Button("Night"))
        timeOfDay = 23.0f;

    ImGui::Separator();
    ImGui::Text("Weather & Lighting");
    if (ImGui::Combo("Weather", &weatherIndex, "Clear\0Rain\0Storm\0Fog\0Sandstorm\0"))
    {
        m_ConsoleHistory.push_back("[World] Weather changed");
    }

    if (ImGui::Checkbox("Global Lighting", &globalLighting))
    {
        m_ConsoleHistory.push_back("[World] Global Lighting: " + std::string(globalLighting ? "ON" : "OFF"));
    }
}

// ==================== SPAWN ====================
void DevMenu::RenderSpawnTab()
{
    ImGui::Text("Spawn Menu");
    ImGui::Separator();

    if (ImGui::Button("Spawn Enemy: Bandit"))
        m_ConsoleHistory.push_back("[Spawn] Bandit spawned");
    if (ImGui::Button("Spawn Enemy: Mutant"))
        m_ConsoleHistory.push_back("[Spawn] Mutant spawned");
    if (ImGui::Button("Spawn NPC: Trader"))
        m_ConsoleHistory.push_back("[Spawn] Trader spawned");
    if (ImGui::Button("Spawn Object: Crate"))
        m_ConsoleHistory.push_back("[Spawn] Crate spawned");
}

// ==================== PLAYER ====================
static bool godMode = false;
static bool noClip = false;
static float moveSpeed = 1.0f;
static float jumpHeight = 1.0f;

void DevMenu::RenderPlayerTab()
{
    ImGui::Text("Player Modes & State");
    ImGui::Separator();

    if (ImGui::Checkbox("God Mode", &godMode))
    {
        m_ConsoleHistory.push_back("[Player] God Mode: " + std::string(godMode ? "ON" : "OFF"));
    }
    if (ImGui::Checkbox("No Clip", &noClip))
    {
        m_ConsoleHistory.push_back("[Player] No Clip: " + std::string(noClip ? "ON" : "OFF"));
    }

    ImGui::Separator();
    if (ImGui::SliderFloat("Move Speed", &moveSpeed, 0.5f, 5.0f))
    {
    }
    if (ImGui::SliderFloat("Jump Height", &jumpHeight, 0.5f, 3.0f))
    {
    }

    ImGui::Separator();
    if (ImGui::Button("Give: 100 Scrap"))
        m_ConsoleHistory.push_back("[Player] +100 Scrap");
    if (ImGui::Button("Give: Assault Rifle"))
        m_ConsoleHistory.push_back("[Player] Gave Assault Rifle");
    if (ImGui::Button("Give: Medkit x5"))
        m_ConsoleHistory.push_back("[Player] Gave Medkits");

    ImGui::Separator();
    if (ImGui::Button("Restore Health & Stamina"))
        m_ConsoleHistory.push_back("[Player] Fully restored");
    if (ImGui::Button("Kill Player"))
        m_ConsoleHistory.push_back("[Player] Player killed");
}

// ==================== MAP TOOLS ====================
void DevMenu::RenderMapToolsTab()
{
    ImGui::Text("Map Layers");
    ImGui::Separator();

    const char* layers[] = {"Ground", "Objects", "Entities", "Triggers", "Effects"};

    for (int i = 0; i < 5; ++i)
    {
        bool layerIsVisible = m_LayerVisibility[i];
        if (ImGui::Checkbox(layers[i], &layerIsVisible))
        {
            SetLayerVisibility(i, layerIsVisible);
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Save Map (.bwld)"))
        SaveMap("maps/current.bwld");
    ImGui::SameLine();
    if (ImGui::Button("Load Map (.bwld)"))
        LoadMap("maps/current.bwld");
}

// ==================== DEBUG ====================
static bool showCollision = false;
static bool showAI = false;
static bool showFPS = true;

void DevMenu::RenderDebugTab()
{
    ImGui::Text("Debug & Performance");
    ImGui::Separator();

    ImGui::Checkbox("Show FPS", &showFPS);
    ImGui::Text("Frame Time: %.2f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("RAM / VRAM: N/A (Р·Р°РіР»СѓС€РєР°)");

    ImGui::Separator();
    if (ImGui::Checkbox("Show Collision / Hitboxes", &showCollision))
    {
        m_ConsoleHistory.push_back("[Debug] Collision: " + std::string(showCollision ? "ON" : "OFF"));
    }
    if (ImGui::Checkbox("Show AI Debug (NavMesh, Vision)", &showAI))
    {
        m_ConsoleHistory.push_back("[Debug] AI Debug: " + std::string(showAI ? "ON" : "OFF"));
    }
}

// ==================== РЎР›РћР ====================
void DevMenu::SetLayerVisibility(int layer, bool isVisible)
{
    m_LayerVisibility[layer] = isVisible;
    m_ConsoleHistory.push_back("[Map] Layer " + std::to_string(layer) + (isVisible ? " ON" : " OFF"));
}

bool DevMenu::IsLayerVisible(int layer) const
{
    auto it = m_LayerVisibility.find(layer);
    return it != m_LayerVisibility.end() ? it->second : true;
}

bool DevMenu::SaveMap(const std::string& filepath)
{
    m_ConsoleHistory.push_back("[Map] Saving to: " + filepath);
    return true;
}

bool DevMenu::LoadMap(const std::string& filepath)
{
    m_ConsoleHistory.push_back("[Map] Loading from: " + filepath);
    return true;
}

#endif // DEV_BUILD
