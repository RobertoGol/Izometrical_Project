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
#include "TimeShift.hpp"
#include "TextureGenerator.hpp"

// ═══════════════════════════════════════════════════════
// Отрисовка изометрического пола
// ═══════════════════════════════════════════════════════
void renderFloor(sf::RenderWindow& window, const bunker::GameState& gs,
                 const bunker::TimeShift& timeShift) {
    for (int x = 0; x < Config::MAP_WIDTH; ++x) {
        for (int y = 0; y < Config::MAP_HEIGHT; ++y) {
            sf::ConvexShape tile(4);
            tile.setPoint(0, bunker::IsoMath::worldToScreen(static_cast<float>(x),     static_cast<float>(y)));
            tile.setPoint(1, bunker::IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y)));
            tile.setPoint(2, bunker::IsoMath::worldToScreen(static_cast<float>(x + 1), static_cast<float>(y + 1)));
            tile.setPoint(3, bunker::IsoMath::worldToScreen(static_cast<float>(x),     static_cast<float>(y + 1)));

            if (gs.sectorMap[x][y] == 1) {
                if (timeShift.isPast()) {
                    tile.setFillColor(sf::Color(60, 70, 90));       // Прошлое — синеватые стены
                    tile.setOutlineColor(sf::Color(80, 90, 115));
                } else {
                    tile.setFillColor(sf::Color(70, 75, 85));       // Настоящее — серые стены
                    tile.setOutlineColor(sf::Color(90, 95, 105));
                }
            } else if (gs.etherErosionMap[x][y] > 5.0f) {
                int intensity = static_cast<int>(std::min(gs.etherErosionMap[x][y] * 1.5f, 80.0f));
                tile.setFillColor(sf::Color(30 + intensity/2, 20, 35 + intensity));
                tile.setOutlineColor(sf::Color(50, 30, 60));
            } else {
                if (timeShift.isPast()) {
                    tile.setFillColor(sf::Color(30, 35, 45));       // Прошлое — холодные тона
                    tile.setOutlineColor(sf::Color(45, 50, 60));
                } else {
                    tile.setFillColor(sf::Color(35, 35, 40));       // Настоящее
                    tile.setOutlineColor(sf::Color(50, 50, 55));
                }
            }

            tile.setOutlineThickness(1.0f);
            window.draw(tile);
        }
    }
}

// ═══════════════════════════════════════════════════════
// Отрисовка сущностей с Z-сортировкой
// ═══════════════════════════════════════════════════════
void renderEntities(sf::RenderWindow& window, const bunker::GameState& gs,
                    const bunker::PlayerController& playerCtrl,
                    const bunker::TimeShift& timeShift) {
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

    // Враги (разные цвета в разных таймлайнах)
    bool isPast = timeShift.isPast();
    for (const auto& e : gs.enemies) {
        if (!e.isAlive) continue;
        float ex = e.position.x, ey = e.position.y, er = e.radius;
        renderQueue.push_back({ ex + ey, [&, ex, ey, er, isPast]() {
            sf::CircleShape shape(8.0f + er * 5.0f, isPast ? 4 : 3);
            shape.setFillColor(isPast ? sf::Color(100, 150, 255) : sf::Color(255, 50, 50));
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
        bool piloted = gs.titan.isPiloted;
        renderQueue.push_back({ bx + by, [&, bx, by, piloted]() {
            sf::CircleShape bt(16.0f, 4);
            bt.setFillColor(piloted ? sf::Color(255, 160, 40) : sf::Color(230, 115, 25));
            bt.setOrigin(16, 16);
            bt.setPosition(bunker::IsoMath::worldToScreen(bx, by));
            bt.setOutlineThickness(2.0f);
            bt.setOutlineColor(sf::Color(180, 80, 10));
            window.draw(bt);
        }});
    }

    // Игрок (только если НЕ в Танке)
    if (gs.playerMode != bunker::UnitMode::Titan) {
        float ppx = gs.playerPos.x, ppy = gs.playerPos.y;
        renderQueue.push_back({ ppx + ppy + 0.01f, [&, ppx, ppy]() {
            sf::CircleShape pShape(12.0f);
            pShape.setFillColor(sf::Color::Cyan);
            pShape.setOrigin(12, 12);
            pShape.setPosition(bunker::IsoMath::worldToScreen(ppx, ppy));
            pShape.setOutlineThickness(2.0f);
            pShape.setOutlineColor(sf::Color(0, 200, 200));
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

    // ── Инициализация систем ──
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
    bunker::TimeShift        timeShift;
    bunker::TextureGenerator texGen;

    // ── Генерация текстур при первом запуске ──
    texGen.generateAllOnFirstRun();

    // ── Загрузка модульных конфигов ──
    vehicleMgr.scanAndLoadConfigs("assets/vehicles");
    enemySpawner.scanAndLoadConfigs("assets/enemies");

    // ── Генерация мира ──
    worldSession.generateDefaultWorld(gs, enemySpawner);

    // ── Инициализация TimeShift (после генерации мира!) ──
    timeShift.initialize(gs, enemySpawner);

    // ── Попытка загрузки сейва ──
    if (bunker::SaveSystem::saveExists(1)) {
        bunker::SaveSystem::readSave(1, gs, inventory);
        std::cout << "[SYSTEM] Сохранение восстановлено." << std::endl;
    }

    // ── Шрифт ──
    sf::Font globalFont;
    bool fontLoaded = globalFont.loadFromFile("assets/fonts/default.ttf");
    hud.loadFont("assets/fonts/default.ttf");
    if (fontLoaded) mapScreen.loadFont(globalFont);

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
        if (dt > 0.1f) dt = 0.1f;
        gs.deltaTime = dt;

        // ── 1. Ввод ──
        bunker::InputSnapshot input = inputMgr.capture(window);

        // Доп. события (зум карты, TimeShift по T, карта по M)
        sf::Event evt;
        while (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) gs.isRunning = false;
            if (mapScreen.isOpen()) mapScreen.handleInput(evt, dt);

            if (evt.type == sf::Event::KeyPressed) {
                if (evt.key.code == sf::Keyboard::M) mapScreen.toggle();
                if (evt.key.code == sf::Keyboard::T) timeShift.tryShift(gs);
            }
        }

        if (input.quit) gs.isRunning = false;

        // ── Карта открыта — пауза геймплея ──
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
        if (input.saveGame) bunker::SaveSystem::writeSave(1, gs, inventory);
        if (input.loadGame) bunker::SaveSystem::readSave(1, gs, inventory);

        // ── 4. Переключение Пилот ↔ Танк (Tab) ──
        if (input.switchMode) {
            if (gs.playerMode == bunker::UnitMode::Scout) {
                if (titanAI.tryMount(gs)) {
                    tactics.enterVehicle();
                }
            } else {
                titanAI.dismount(gs);
                tactics.exitVehicle();
            }
        }

        // ── 5. Смена класса пилота (1-7, только Scout) ──
        if (gs.playerMode == bunker::UnitMode::Scout && !vehicleMgr.isPlayerInVehicle()) {
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

        // TimeShift
        timeShift.update(dt);

        // Транспорт
        if (vehicleMgr.isPlayerInVehicle()) {
            vehicleMgr.update(gs, input, dt);
        } else if (gs.playerMode == bunker::UnitMode::Scout) {
            // Пешком
            playerCtrl.update(gs, input, dt);

            // Посадка в транспорт или взаимодействие с контейнером (E)
            if (input.interact) {
                if (!vehicleMgr.mountNearest(gs)) {
                    worldSession.interactWithContainers(gs, inventory);
                }
            }
        }

        // Тактики (только Scout)
        tactics.updateCooldowns(gs, dt);
        if (input.activateTactical && gs.playerMode == bunker::UnitMode::Scout
            && !vehicleMgr.isPlayerInVehicle()) {
            tactics.activateTactical(gs, gs.mouseWorldPos);
        }
        tactics.processGrapplePhysics(gs, dt);

        // Танк БТ-7274 (всегда обновляется — и автономный и пилотируемый)
        titanAI.update(gs, input, dt);

        // Стрельба
        if (input.isShooting && gs.fireCooldown <= 0.0f) {
            if (gs.playerMode == bunker::UnitMode::Scout && !vehicleMgr.isPlayerInVehicle()) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::F10)) {
                    bulletSys.fireDebugGunChainLightning(gs);
                } else {
                    bulletSys.fireScoutWeapon(gs, gs.isAiming);
                }
            } else if (gs.playerMode == bunker::UnitMode::Titan) {
                titanAI.fireFromCockpit(gs);
            }
        }

        // Ракеты Титана (СКМ)
        if (input.isMissile && gs.playerMode == bunker::UnitMode::Titan) {
            bulletSys.fireTitanMissiles(gs);
        }

        // Автострельба Танка (автономный режим)
        titanAI.autoFire(gs);

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

        camera.applyTo(window);

        // Пол + стены (с учётом таймлайна)
        renderFloor(window, gs, timeShift);

        // Сущности (Z-sort, разные цвета врагов по таймлайну)
        renderEntities(window, gs, playerCtrl, timeShift);

        // Пули + молнии
        bulletSys.render(gs, window, camera.getView());

        // ── HUD (экранные координаты) ──
        window.setView(window.getDefaultView());

        hud.render(window, gs, playerCtrl, tactics, titanAI, vehicleMgr, inventory);

        // TimeShift HUD (индикатор таймлайна + заряд)
        if (timeShift.isInitialized()) {
            timeShift.renderHUD(window, fontLoaded ? &globalFont : nullptr);
            timeShift.renderTransitionEffect(window);
        }

        window.display();
    }

    // ── Автосохранение при выходе ──
    bunker::SaveSystem::writeSave(1, gs, inventory);
    std::cout << "[SYSTEM] Автосохранение при выходе. До встречи, Пилот." << std::endl;

    return 0;
}
