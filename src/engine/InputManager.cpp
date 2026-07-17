#include "engine/InputManager.hpp"

namespace bunker
{

    InputSnapshot InputManager::capture(sf::RenderWindow& window)
    {
        InputSnapshot snap;

        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                snap.quit = true;
            }
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch (keyPressed->code)
                {
                case sf::Keyboard::Key::Escape:
                    snap.quit = true;
                    break;
                case sf::Keyboard::Key::Tab:
                    snap.switchMode = true;
                    break;
                case sf::Keyboard::Key::R:
                    snap.switchWeapon = true;
                    break;
                case sf::Keyboard::Key::X:
                    snap.dismountVehicle = true;
                    break;
                case sf::Keyboard::Key::B:
                    snap.toggleCamp = true;
                    break;
                case sf::Keyboard::Key::E:
                    snap.interact = true;
                    break;
                case sf::Keyboard::Key::Q:
                    snap.activateTactical = true;
                    break;
                case sf::Keyboard::Key::F5:
                    snap.saveGame = true;
                    break;
                case sf::Keyboard::Key::F9:
                    snap.loadGame = true;
                    break;
                case sf::Keyboard::Key::Num1:
                    snap.selectClass[0] = true;
                    break;
                case sf::Keyboard::Key::Num2:
                    snap.selectClass[1] = true;
                    break;
                case sf::Keyboard::Key::Num3:
                    snap.selectClass[2] = true;
                    break;
                case sf::Keyboard::Key::Num4:
                    snap.selectClass[3] = true;
                    break;
                case sf::Keyboard::Key::Num5:
                    snap.selectClass[4] = true;
                    break;
                case sf::Keyboard::Key::Num6:
                    snap.selectClass[5] = true;
                    break;
                case sf::Keyboard::Key::Num7:
                    snap.selectClass[6] = true;
                    break;
                default:
                    break;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            snap.moveForward += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            snap.moveForward -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            snap.moveStrafe -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            snap.moveStrafe += 1.0f;

        snap.isSprinting = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);
        snap.isDiving = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        snap.isAiming = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        snap.isShooting = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        snap.isMissile = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
        snap.mousePixelPos = sf::Mouse::getPosition(window);

        return snap;
    }

} // namespace bunker
