#pragma once

// В dev-сборке раскомментируй эту строку или передавай через CMake: -DDEV_BUILD=ON
// #define DEV_BUILD

#ifdef DEV_BUILD

#include <map>
#include <string>
#include <vector>

class DevMenu
{
public:
    static DevMenu &Get();

    void Initialize();
    void Shutdown();

    void Toggle();
    void Update();
    void Render();

    bool IsEnabled() const;
    bool IsVisible() const;

private:
    DevMenu() = default;

    void LoadScripts();
    void ExecuteLuaCommand(const std::string &cmd);
    void RenderConsoleTab();
    void ExecuteCommand(const std::string &cmd);
    void RenderWorldTab();
    void RenderSpawnTab();
    void RenderPlayerTab();
    void RenderMapToolsTab();
    void RenderDebugTab();
    void SetLayerVisibility(int layer, bool isVisible);
    bool IsLayerVisible(int layer) const;
    bool SaveMap(const std::string &filepath);
    bool LoadMap(const std::string &filepath);

    bool visible = false;
    bool m_LuaInitialized = false;
    std::string m_ConsoleInput;
    std::vector<std::string> m_ConsoleHistory;
    std::map<int, bool> m_LayerVisibility;
};

#endif // DEV_BUILD
