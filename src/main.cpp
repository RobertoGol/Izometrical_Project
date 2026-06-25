#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>

// ── Все системы движка ──
#include "Constants.hpp"
#include "Types.hpp"
#include "IsoMath.hpp"
#include "GameState.hpp"
#include "Camera.hpp"
#include "InputManager.hpp"
#include "PlayerController.hpp"
#include "Collisions.hpp"
#include "Tactics.hpp"
#include "BulletSystem.hpp"
#include "TitanAI.hpp"
#include "VehicleManager.hpp"
#include "EnemySpawner.hpp"
#include "WorldSession.hpp"
#include "Inventory.hpp"
#include "SaveSystem.hpp"
#include "Progression.hpp"
#include "HUD.hpp"
#include "MapScreen.hpp"

// ═══════════════════════════════════════════════════════
// Отрисовка изометрического пола
// ═══════════════════════════════════════════════════════
void renderFloor(sf::RenderWindow& window, const bunker::GameState& gs) {
    for (int x = 0; x < Config::MAP_WIDTH; ++x) {
        for (int y = 0; y < Config::MAP_HEIGHT; ++y) {
            sf::ConvexShape tile(4);
            tile.setPoint(0, bunker::IsoMath::worldToScreen(static_cast<float>(x),     static_cast<float>(y)));
            tile.setPoint(1, bunker::IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y)));
            tile.setPoint(2, bunker::IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y + 1)));
            tile.setPoint(3, bunker::IsoMath::worldToScreen(static_cast<float>(x),     static_cast<float>(y + 1)));

            if (gs.sectorMap[x][y] == 1) {
                // Стена
                tile.setFillColor(sf::Color(70, 75, 85));
                tile.setOutlineColor(sf::Color(90, 95, 105));
            } else if (gs.etherErosionMap[x][y] > 5.0f) {
                // Эрозия
                int intensity = static_cast<int>(std::min(gs.etherErosionMap[x][y] * 1.5f, 80.0f));
                tile.setFillColor(sf::Color(30 + intensity/2, 20, 35 + intensity));
                tile.setOutlineColor(sf::Color(50, 30, 60));
            } else {
                // Обычный пол
                tile.setFillColor(sf::Color(35, 35, 40));
                tile.setOutlineColor(sf::Color(50, 50, 55));
            }

            tile.setOutlineThickness(1.0f);
            window.draw(tile);
        }
    }
}

// ═══════════════════════════════════════════════════════
// Отрисовка игровых объектов с Z-сортировкой
// ═══════════════════════════════════════════════════════
void renderEntities(sf::RenderWindow& window, const bunker::GameState& gs,
                    const bunker::PlayerController& playerCtrl) {
    std::vector<bunker::RenderObject> renderQueue;

    // Pip-Pad на полу
    if (!gs.bunkerProgression.hasFoundPipPad) {
        float px = gs.bunkerProgression.pipPadSpawnPos.x;
        float py = gs.bunkerProgression.pipPadSpawnPos.y;
        renderQueue.push_back({ px + py, [&, px, py]() {
            sf::RectangleShape pad({10, 8});
            pad.setFillColor(sf::Color(255, 255, 50, 200));
            pad.setOrigin(5, 4);
            pad.setPosition(bunker::IsoMath::worldToScreen(px, py));
            window.draw(pad);
        }});
    }

    // Лут-контейнеры
    for (const auto& c : gs.lootContainers) {
        float cx = c.position.x, cy = c.position.y;
        bool opened = c.isOpened;
        renderQueue.push_back({ cx + cy, [&, cx, cy, opened]() {
            sf::RectangleShape box({12, 10});
            box.setOrigin(6, 5);
            box.setPosition(bunker::IsoMath::worldToScreen(cx, cy));
            box.setFillColor(opened ? sf::Color(80, 60, 30, 120) : sf::Color(180, 140, 60));
            box.setOutlineThickness(1.0f);
            box.setOutlineColor(sf::Color(120, 100, 40));
            window.draw(box);
        }});
    }

    // Враги
    for (const auto& e : gs.enemies) {
        if (!e.isAlive) continue;
        float ex = e.position.x, ey = e.position.y;
        float er = e.radius;
        renderQueue.push_back({ ex + ey, [&, ex, ey, er]() {
            sf::CircleShape shape(8.0f + er * 5.0f, 3);
            shape.setFillColor(sf::Color(255, 50, 50));
            shape.setOrigin(8.0f + er * 5.0f, 8.0f + er * 5.0f);
            shape.setPosition(bunker::IsoMath::worldToScreen(ex, ey));
            window.draw(shape);
        }});
    }

    // Вышка связи
    {
        float tx = gs.towerPosition.x, ty = gs.towerPosition.y;
        renderQueue.push_back({ tx + ty, [&, tx, ty]() {
            sf::CircleShape tower(7.0f, 6);
            tower.setFillColor(sf::Color(200, 200, 255));
            tower.setOrigin(7, 7);
            tower.setPosition(bunker::IsoMath::worldToScreen(tx, ty));
            tower.setOutlineThickness(2.0f);
            tower.setOutlineColor(sf::Color::White);
            window.draw(tower);
        }});
    }

    // Танк БТ-7274
    {
        float bx = gs.titan.position.x, by = gs.titan.position.y;
        renderQueue.push_back({ bx + by, [&, bx, by]() {
            sf::CircleShape bt(16.0f, 4);
            bt.setFillColor(sf::Color(230, 115, 25));
            bt.setOrigin(16, 16);
            bt.setPosition(bunker::IsoMath::worldToScreen(bx, by));
            bt.setOutlineThickness(2.0f);
            bt.setOutlineColor(sf::Color(180, 80, 10));
            window.draw(bt);
        }});
    }

    // Игрок
    {
        float ppx = gs.playerPos.x, ppy = gs.playerPos.y;
        bool isTitan = (gs.playerMode == bunker::UnitMode::Titan);
        renderQueue.push_back({ ppx + ppy + 0.01f, [&, ppx, ppy, isTitan]() {
            float r = isTitan ? 16.0f : 12.0f;
            sf::CircleShape pShape(r);
            pShape.setFillColor(isTitan ? sf::Color(230, 140, 40) : sf::Color::Cyan);
            pShape.setOrigin(r, r);
            pShape.setPosition(bunker::IsoMath::worldToScreen(ppx, ppy));
            pShape.setOutlineThickness(2.0f);
            pShape.setOutlineColor(isTitan ? sf::Color(200, 100, 10) : sf::Color(0, 200, 200));
            window.draw(pShape);
        }});
    }

    // Z-сортировка и отрисовка
    std::sort(renderQueue.begin(), renderQueue.end(),
        [](const bunker::RenderObject& a, const bunker::RenderObject& b) {
            return a.depth < b.depth;
        });

    for (auto& obj : renderQueue) {
        obj.drawFunc();
    }
}

// ═══════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════
int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // ── Окно ──
    sf::RenderWindow window(
        sf::VideoMode(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT),
        "Bunker Protocol ISO",
        sf::Style::Close | sf::Style::Titlebar
    );
    window.setFramerateLimit(60);

    // ── Инициализация всех систем ──
    bunker::GameState        gs;
    bunker::InputManager     inputMgr;
    bunker::Camera           camera;
    bunker::PlayerController playerCtrl;
    bunker::TacticsManager   tactics;
    bunker::BulletSystem     bulletSys;
    bunker::TitanAI          titanAI;
    bunker::VehicleManager   vehicleMgr;
    bunker::EnemySpawner     enemySpawner;
    bunker::WorldSession     worldSession;
    bunker::PlayerInventory  inventory;
    bunker::HUD              hud;
    bunker::MapScreen        mapScreen;

    // ── Загрузка модульных конфигов ──
    vehicleMgr.scanAndLoadConfigs("assets/vehicles");
    enemySpawner.scanAndLoadConfigs("assets/enemies");

    // ── Генерация мира ──
    worldSession.generateDefaultWorld(gs, enemySpawner);

    // ── Попытка загрузки сейва ──
    if (bunker::SaveSystem::saveExists(1)) {
        bunker::SaveSystem::readSave(1, gs, inventory);
        std::cout << "[SYSTEM] Сохранение восстановлено." << std::endl;
    }

    // ── Шрифт ──
    hud.loadFont("assets/fonts/default.ttf");
    sf::Font globalFont;
    if (globalFont.loadFromFile("assets/fonts/default.ttf")) {
        mapScreen.loadFont(globalFont);
    }

    // ── Спавн транспорта на карту ──
    vehicleMgr.spawnVehicle("steamcar",   {3.0f, 3.0f, 0.0f});
    vehicleMgr.spawnVehicle("motorcycle", {16.0f, 3.0f, 0.0f});

    sf::Clock clock;
    std::cout << "[SYSTEM] Bunker Protocol ISO запущен." << std::endl;

    // ═══════════════════════════════════════════════
    // ГЛАВНЫЙ ИГРОВОЙ ЦИКЛ
    // ═══════════════════════════════════════════════
    while (window.isOpen() && gs.isRunning) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;  // Защита от лагов
        gs.deltaTime = dt;

        // ── 1. Ввод ──
        bunker::InputSnapshot input = inputMgr.capture(window);

        // Обработка событий для карты (зум колёсиком)
        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) gs.isRunning = false;
            if (mapScreen.isOpen()) mapScreen.handleInput(evt, dt);
            // Клавиша M — переключение карты
            if (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::M) {
                mapScreen.toggle();
            }
        }

        if (input.quit) gs.isRunning = false;

        // ── Если карта открыта — только скролл, без геймплея ──
        if (mapScreen.isOpen()) {
            mapScreen.updatePan(dt);

            window.clear(sf::Color(20, 20, 22));
            mapScreen.render(window, gs);
            window.display();
            continue;
        }

        // ── 2. Мышь → мировые координаты ──
        gs.mouseWorldPos = camera.screenToWorld(window, input.mousePixelPos);

        // ── 3. Сохранение / Загрузка ──
        if (input.saveGame) {
            bunker::SaveSystem::writeSave(1, gs, inventory);
        }
        if (input.loadGame) {
            bunker::SaveSystem::readSave(1, gs, inventory);
        }

        // ── 4. Переключение Пилот ↔ Танк (Tab) ──
        if (input.switchMode) {
            if (gs.playerMode == bunker::UnitMode::Scout) {
                // Проверяем расстояние до Танка
                float dx = gs.playerPos.x - gs.titan.position.x;
                float dy = gs.playerPos.y - gs.titan.position.y;
                if ((dx * dx + dy * dy) < 4.0f) {
                    gs.playerMode = bunker::UnitMode::Titan;
                    gs.titan.isPiloted = true;
                    gs.playerPos = gs.titan.position;
                    tactics.enterVehicle();
                }
            } else {
                gs.playerMode = bunker::UnitMode::Scout;
                gs.titan.isPiloted = false;
                gs.playerPos.x = gs.titan.position.x + 1.0f;
                tactics.exitVehicle();
            }
        }

        // ── 5. Смена класса пилота (1-7) ──
        if (gs.playerMode == bunker::UnitMode::Scout) {
            bunker::PilotClass classes[] = {
                bunker::PilotClass::Grapple, bunker::PilotClass::Cloak,
                bunker::PilotClass::Stim, bunker::PilotClass::PhaseShift,
                bunker::PilotClass::HoloPilot, bunker::PilotClass::AWall,
                bunker::PilotClass::PulseBlade
            };
            for (int i = 0; i < 7; ++i) {
                if (input.selectClass[i]) tactics.changePilotClass(classes[i]);
            }
        }

        // ── 6. Обновление систем ──

        // Транспорт
        if (vehicleMgr.isPlayerInVehicle()) {
            vehicleMgr.update(gs, input, dt);
        } else if (gs.playerMode == bunker::UnitMode::Scout) {
            // Пешком
            playerCtrl.update(gs, input, dt);

            // Посадка в транспорт (E)
            if (input.interact) {
                if (!vehicleMgr.mountNearest(gs)) {
                    // Если не транспорт — пробуем контейнер
                    worldSession.interactWithContainers(gs, inventory);
                }
            }
        }

        // Тактики
        tactics.updateCooldowns(gs, dt);
        if (input.activateTactical && gs.playerMode == bunker::UnitMode::Scout) {
            tactics.activateTactical(gs, gs.mouseWorldPos);
        }
        tactics.processGrapplePhysics(gs, dt);

        // Танк БТ-7274
        titanAI.update(gs, input, dt);
        if (gs.titan.fireCooldown > 0.0f)
            gs.titan.fireCooldown -= dt;

        // Стрельба
        if (input.isShooting && gs.fireCooldown <= 0.0f) {
            if (gs.playerMode == bunker::UnitMode::Scout && !vehicleMgr.isPlayerInVehicle()) {
                // DebugGun (F10 зажат)
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::F10)) {
                    bulletSys.fireDebugGunChainLightning(gs);
                } else {
                    bulletSys.fireScoutWeapon(gs, gs.isAiming);
                }
            } else if (gs.playerMode == bunker::UnitMode::Titan) {
                bulletSys.fireTitanWeapon(gs);
            }
        }

        // Ракеты Титана (СКМ)
        if (input.isMissile && gs.playerMode == bunker::UnitMode::Titan) {
            bulletSys.fireTitanMissiles(gs);
        }

        // Автострельба Титана
        bulletSys.titanAutoFire(gs);

        // Баллистика
        bulletSys.update(gs, dt);

        // Враги
        enemySpawner.updateWaveSpawning(gs, dt);
        enemySpawner.updateEnemyAI(gs, dt);

        // Коллизии
        bunker::Collisions::resolveAllCollisions(gs);

        // Мир (эрозия, осада, Pip-Pad)
        worldSession.update(gs, dt);

        // Камера
        camera.update(gs.playerPos, gs.mouseWorldPos, gs.isAiming, dt);

        // ── 7. Рендер ──
        window.clear(sf::Color(20, 20, 22));

        // Игровая камера
        camera.applyTo(window);

        // Пол + стены
        renderFloor(window, gs);

        // Сущности (Z-sort)
        renderEntities(window, gs, playerCtrl);

        // Пули + молнии
        bulletSys.render(gs, window, camera.getView());

        // Сброс View на экранный для HUD
        window.setView(window.getDefaultView());

        // HUD
        hud.render(window, gs, playerCtrl, tactics, titanAI, vehicleMgr, inventory);

        window.display();
    }

    // ── Автосохранение при выходе ──
    bunker::SaveSystem::writeSave(1, gs, inventory);
    std::cout << "[SYSTEM] Автосохранение при выходе. До встречи, Пилот." << std::endl;

    return 0;
}
