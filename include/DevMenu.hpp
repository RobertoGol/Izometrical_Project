#pragma once

// Раскомментируй эту строку в dev-сборке
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

    bool visible = false;
};

#endif // ENABLE_DEVMENU