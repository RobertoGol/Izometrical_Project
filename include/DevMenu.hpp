#pragma once

// В dev-сборке раскомментируй эту строку или передавай через CMake: -DDEV_BUILD=ON
// #define DEV_BUILD

#ifdef DEV_BUILD

#include <string>

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

    bool visible = false;
};

#endif // DEV_BUILD