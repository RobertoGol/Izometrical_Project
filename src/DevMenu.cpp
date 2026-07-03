#include "DevMenu.hpp"

#ifdef ENABLE_DEVMENU

#include <iostream>

DevMenu &DevMenu::Get()
{
    static DevMenu instance;
    return instance;
}

void DevMenu::Initialize()
{
    std::cout << "[DevMenu] DevMenu initialized (ENABLE_DEVMENU defined)" << std::endl;
    LoadScripts();
}

void DevMenu::Shutdown()
{
}

void DevMenu::Toggle()
{
    visible = !visible;
}

void DevMenu::Update()
{
}

void DevMenu::Render()
{
    if (!visible)
        return;

    // TODO: Рендер через ImGui + Lua
}

void DevMenu::LoadScripts()
{
    std::cout << "[DevMenu] Loading scripts from dev/ folder..." << std::endl;
    // TODO: Загрузка Lua скриптов
}

bool DevMenu::IsEnabled() const
{
    return true;
}

bool DevMenu::IsVisible() const
{
    return visible;
}

#endif // DEV_BUILD