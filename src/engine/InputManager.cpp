#include "engine/InputManager.hpp"

namespace bunker
{

    InputSnapshot InputManager::capture(sf::RenderWindow &window)
    {
        InputSnapshot snap;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                snap.quit = true;
            }
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    snap.quit = true;
                    break;
                case sf::Keyboard::Tab:
                    snap.switchMode = true;
                    break;
                case sf::Keyboard::R:
                    snap.switchWeapon = true;
                    break;
                case sf::Keyboard::X:
                    snap.dismountVehicle = true;
                    break;
                case sf::Keyboard::B:
                    snap.toggleCamp = true;
                    break;
                case sf::Keyboard::E:
                    snap.interact = true;
                    break;
                case sf::Keyboard::Q:
                    snap.activateTactical = true;
                    break;
                case sf::Keyboard::F5:
                    snap.saveGame = true;
                    break;
                case sf::Keyboard::F9:
                    snap.loadGame = true;
                    break;
                case sf::Keyboard::Num1:
                    snap.selectClass[0] = true;
                    break;
                case sf::Keyboard::Num2:
                    snap.selectClass[1] = true;
                    break;
                case sf::Keyboard::Num3:
                    snap.selectClass[2] = true;
                    break;
                case sf::Keyboard::Num4:
                    snap.selectClass[3] = true;
                    break;
                case sf::Keyboard::Num5:
                    snap.selectClass[4] = true;
                    break;
                case sf::Keyboard::Num6:
                    snap.selectClass[5] = true;
                    break;
                case sf::Keyboard::Num7:
                    snap.selectClass[6] = true;
                    break;
                default:
                    break;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            snap.moveForward += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            snap.moveForward -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            snap.moveStrafe -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            snap.moveStrafe += 1.0f;

        snap.isSprinting = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
        snap.isDiving = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

        snap.isAiming = sf::Mouse::isButtonPressed(sf::Mouse::Right);
        snap.isShooting = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        snap.isMissile = sf::Mouse::isButtonPressed(sf::Mouse::Middle);
        snap.mousePixelPos = sf::Mouse::getPosition(window);

        return snap;
    }

} // namespace bunker
