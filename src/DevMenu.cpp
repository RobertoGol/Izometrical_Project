#include "DevMenu.hpp"

#ifdef DEV_BUILD

#include "engine/Log.hpp"
#include <imgui.h>
#include <string>
#include <vector>

// ==================== LUA (заглушка) ====================
// В будущем здесь будет полноценная интеграция через sol2 или lua.hpp
// Пока просто заглушка для архитектуры

static bool luaInitialized = false;

void DevMenu::LoadScripts()
{
    bunker::logInfo() << "[DevMenu] Loading Lua scripts from dev/..." << std::endl;
    // TODO: Загрузка dev/DevMenu.lua
    luaInitialized = true;
}

void DevMenu::ExecuteLuaCommand(const std::string& cmd)
{
    if (!luaInitialized)
    {
        consoleHistory.push_back("[Lua] Lua not initialized");
        return;
    }
    // TODO: Выполнение Lua кода
    consoleHistory.push_back("[Lua] Executed: " + cmd);
}

// ==================== ОСТАЛЬНОЙ КОД ====================

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
        layerVisibility[i] = true;
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

// ==================== CONSOLE ====================
static std::string consoleInput;
static std::vector<std::string> consoleHistory;

void DevMenu::RenderConsoleTab()
{
    ImGui::Text("Console Commands");
    ImGui::Separator();

    if (ImGui::InputText("##cmd", &consoleInput, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (!consoleInput.empty())
        {
            ExecuteCommand(consoleInput);
            consoleInput.clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Execute"))
    {
        if (!consoleInput.empty())
        {
            ExecuteCommand(consoleInput);
            consoleInput.clear();
        }
    }

    ImGui::BeginChild("History", ImVec2(0, 500), true);
    for (const auto& line : consoleHistory)
    {
        ImGui::TextUnformatted(line.c_str());
    }
    ImGui::EndChild();
}

void DevMenu::ExecuteCommand(const std::string& cmd)
{
    consoleHistory.push_back("> " + cmd);

    if (cmd.find("lua ") == 0)
    {
        ExecuteLuaCommand(cmd.substr(4));
    }
    else if (cmd.find("place tile") == 0)
    {
        consoleHistory.push_back("[Console] Tile placed");
    }
    else if (cmd.find("spawn") == 0)
    {
        consoleHistory.push_back("[Console] Entity spawned");
    }
    else if (cmd == "help")
    {
        consoleHistory.push_back("Commands: place tile, spawn, set layer, time, weather, god, noclip, lua <code>");
    }
    else if (cmd == "clear")
    {
        consoleHistory.clear();
    }
    else
    {
        consoleHistory.push_back("[Console] Unknown command");
    }
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
        consoleHistory.push_back("[World] Time Scale: " + std::to_string(timeScale));
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
        consoleHistory.push_back("[World] Weather changed");
    }

    if (ImGui::Checkbox("Global Lighting", &globalLighting))
    {
        consoleHistory.push_back("[World] Global Lighting: " + std::string(globalLighting ? "ON" : "OFF"));
    }
}

// ==================== SPAWN ====================
void DevMenu::RenderSpawnTab()
{
    ImGui::Text("Spawn Menu");
    ImGui::Separator();

    if (ImGui::Button("Spawn Enemy: Bandit"))
        consoleHistory.push_back("[Spawn] Bandit spawned");
    if (ImGui::Button("Spawn Enemy: Mutant"))
        consoleHistory.push_back("[Spawn] Mutant spawned");
    if (ImGui::Button("Spawn NPC: Trader"))
        consoleHistory.push_back("[Spawn] Trader spawned");
    if (ImGui::Button("Spawn Object: Crate"))
        consoleHistory.push_back("[Spawn] Crate spawned");
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
        consoleHistory.push_back("[Player] God Mode: " + std::string(godMode ? "ON" : "OFF"));
    }
    if (ImGui::Checkbox("No Clip", &noClip))
    {
        consoleHistory.push_back("[Player] No Clip: " + std::string(noClip ? "ON" : "OFF"));
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
        consoleHistory.push_back("[Player] +100 Scrap");
    if (ImGui::Button("Give: Assault Rifle"))
        consoleHistory.push_back("[Player] Gave Assault Rifle");
    if (ImGui::Button("Give: Medkit x5"))
        consoleHistory.push_back("[Player] Gave Medkits");

    ImGui::Separator();
    if (ImGui::Button("Restore Health & Stamina"))
        consoleHistory.push_back("[Player] Fully restored");
    if (ImGui::Button("Kill Player"))
        consoleHistory.push_back("[Player] Player killed");
}

// ==================== MAP TOOLS ====================
void DevMenu::RenderMapToolsTab()
{
    ImGui::Text("Map Layers");
    ImGui::Separator();

    const char* layers[] = {"Ground", "Objects", "Entities", "Triggers", "Effects"};

    for (int i = 0; i < 5; ++i)
    {
        bool visible = layerVisibility[i];
        if (ImGui::Checkbox(layers[i], &visible))
        {
            SetLayerVisibility(i, visible);
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
    ImGui::Text("RAM / VRAM: N/A (заглушка)");

    ImGui::Separator();
    if (ImGui::Checkbox("Show Collision / Hitboxes", &showCollision))
    {
        consoleHistory.push_back("[Debug] Collision: " + std::string(showCollision ? "ON" : "OFF"));
    }
    if (ImGui::Checkbox("Show AI Debug (NavMesh, Vision)", &showAI))
    {
        consoleHistory.push_back("[Debug] AI Debug: " + std::string(showAI ? "ON" : "OFF"));
    }
}

// ==================== СЛОИ ====================
void DevMenu::SetLayerVisibility(int layer, bool visible)
{
    layerVisibility[layer] = visible;
    consoleHistory.push_back("[Map] Layer " + std::to_string(layer) + (visible ? " ON" : " OFF"));
}

bool DevMenu::IsLayerVisible(int layer) const
{
    auto it = layerVisibility.find(layer);
    return it != layerVisibility.end() ? it->second : true;
}

bool DevMenu::SaveMap(const std::string& filepath)
{
    consoleHistory.push_back("[Map] Saving to: " + filepath);
    return true;
}

bool DevMenu::LoadMap(const std::string& filepath)
{
    consoleHistory.push_back("[Map] Loading from: " + filepath);
    return true;
}

#endif // DEV_BUILD