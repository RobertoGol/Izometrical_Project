#include "DevMenu.hpp"

#ifdef DEV_BUILD

#include "engine/Log.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void DevMenu::LoadScripts()
{
    bunker::logInfo() << "[DevMenu] Loading Lua scripts from dev/..." << std::endl;
    m_LuaInitialized = true;
}

void DevMenu::ExecuteLuaCommand(const std::string& cmd)
{
    if (!m_LuaInitialized)
    {
        m_ConsoleHistory.push_back("[Lua] Lua not initialized");
        return;
    }
    m_ConsoleHistory.push_back("[Lua] Executed: " + cmd);
}

void DevMenu::RenderConsoleTab()
{
    ImGui::Text("Console Commands");
    ImGui::Separator();

    if (ImGui::InputText("##cmd", &m_ConsoleInput, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (!m_ConsoleInput.empty())
        {
            ExecuteCommand(m_ConsoleInput);
            m_ConsoleInput.clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Execute"))
    {
        if (!m_ConsoleInput.empty())
        {
            ExecuteCommand(m_ConsoleInput);
            m_ConsoleInput.clear();
        }
    }

    ImGui::BeginChild("History", ImVec2(0, 500), true);
    for (const auto& line : m_ConsoleHistory)
    {
        ImGui::TextUnformatted(line.c_str());
    }
    ImGui::EndChild();
}

void DevMenu::ExecuteCommand(const std::string& cmd)
{
    m_ConsoleHistory.push_back("> " + cmd);

    if (cmd.find("lua ") == 0)
    {
        ExecuteLuaCommand(cmd.substr(4));
    }
    else if (cmd.find("place tile") == 0)
    {
        m_ConsoleHistory.push_back("[Console] Tile placed");
    }
    else if (cmd.find("spawn") == 0)
    {
        m_ConsoleHistory.push_back("[Console] Entity spawned");
    }
    else if (cmd == "help")
    {
        m_ConsoleHistory.push_back("Commands: place tile, spawn, set layer, time, weather, god, noclip, lua <code>");
    }
    else if (cmd == "clear")
    {
        m_ConsoleHistory.clear();
    }
    else
    {
        m_ConsoleHistory.push_back("[Console] Unknown command");
    }
}

#endif // DEV_BUILD
