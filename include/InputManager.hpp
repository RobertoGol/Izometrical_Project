#pragma once

#include <SFML/Graphics.hpp>

namespace bunker {

// Снимок состояния ввода за текущий кадр.
// Заполняется один раз в начале кадра, затем передаётся во все системы.

struct InputSnapshot {
    // ── Движение ──
    float moveForward = 0.0f;   // +1 = W, -1 = S
    float moveStrafe  = 0.0f;   // +1 = D, -1 = A

    // ── Действия ──
    bool isSprinting  = false;  // Shift
    bool isDiving     = false;  // Space (нырок Helldivers)
    bool isAiming     = false;  // ПКМ (правая кнопка мыши)
    bool isShooting   = false;  // ЛКМ
    bool isMissile    = false;  // СКМ (колёсико)

    // ── Тактика ──
    bool activateTactical = false;  // Q

    // ── Переключения ──
    bool switchMode       = false;  // Tab (Пилот ↔ Титан)
    bool switchWeapon     = false;  // R (переключить оружие Титана)
    bool dismountVehicle  = false;  // X (выйти из транспорта)
    bool toggleCamp       = false;  // B (режим строительства C.A.M.P.)
    bool interact         = false;  // E (взаимодействие / подбор)

    // ── Классы пилота (1–7) ──
    bool selectClass[7]   = {};     // 1=Grapple, 2=Cloak, 3=Stim, 4=Phase, 5=Holo, 6=AWall, 7=Pulse

    // ── Мышь (экранные пиксели) ──
    sf::Vector2i mousePixelPos = {0, 0};

    // ── Системные ──
    bool saveGame  = false;  // F5
    bool loadGame  = false;  // F9
    bool quit      = false;  // Escape
};

class InputManager {
public:
    InputManager() = default;

    // Вызывается один раз в начале кадра.
    // Обрабатывает события SFML + опрашивает зажатые клавиши.
    InputSnapshot capture(sf::RenderWindow& window) {
        InputSnapshot snap;

        // ── Обработка событий SFML (закрытие, одиночные нажатия) ──
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                snap.quit = true;
            }
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape: snap.quit = true; break;
                    case sf::Keyboard::Tab:    snap.switchMode = true; break;
                    case sf::Keyboard::R:      snap.switchWeapon = true; break;
                    case sf::Keyboard::X:      snap.dismountVehicle = true; break;
                    case sf::Keyboard::B:      snap.toggleCamp = true; break;
                    case sf::Keyboard::E:      snap.interact = true; break;
                    case sf::Keyboard::Q:      snap.activateTactical = true; break;
                    case sf::Keyboard::F5:     snap.saveGame = true; break;
                    case sf::Keyboard::F9:     snap.loadGame = true; break;
                    case sf::Keyboard::Num1:   snap.selectClass[0] = true; break;
                    case sf::Keyboard::Num2:   snap.selectClass[1] = true; break;
                    case sf::Keyboard::Num3:   snap.selectClass[2] = true; break;
                    case sf::Keyboard::Num4:   snap.selectClass[3] = true; break;
                    case sf::Keyboard::Num5:   snap.selectClass[4] = true; break;
                    case sf::Keyboard::Num6:   snap.selectClass[5] = true; break;
                    case sf::Keyboard::Num7:   snap.selectClass[6] = true; break;
                    default: break;
                }
            }
        }

        // ── Опрос зажатых клавиш (каждый кадр) ──
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) snap.moveForward += 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) snap.moveForward -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) snap.moveStrafe  -= 1.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) snap.moveStrafe  += 1.0f;

        snap.isSprinting = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
        snap.isDiving    = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

        // ── Мышь ──
        snap.isAiming  = sf::Mouse::isButtonPressed(sf::Mouse::Right);
        snap.isShooting = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        snap.isMissile = sf::Mouse::isButtonPressed(sf::Mouse::Middle);
        snap.mousePixelPos = sf::Mouse::getPosition(window);

        return snap;
    }
};

}  // namespace bunker
