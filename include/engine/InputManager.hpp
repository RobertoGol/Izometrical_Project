#pragma once

#include <SFML/Graphics.hpp>

namespace bunker
{

    struct InputSnapshot
    {
        float moveForward = 0.0f;
        float moveStrafe = 0.0f;
        bool isSprinting = false;
        bool isDiving = false;
        bool isAiming = false;
        bool isShooting = false;
        bool isMissile = false;
        bool activateTactical = false;
        bool switchMode = false;
        bool switchWeapon = false;
        bool dismountVehicle = false;
        bool toggleCamp = false;
        bool interact = false;
        bool selectClass[7] = {};
        sf::Vector2i mousePixelPos = {0, 0};
        bool saveGame = false;
        bool loadGame = false;
        bool quit = false;
    };

    class InputManager
    {
    public:
        InputManager() = default;
        InputSnapshot capture(sf::RenderWindow &window);
    };

} // namespace bunker
