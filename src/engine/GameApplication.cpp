#include "engine/GameApplication.hpp"
#include "content/MeshBuilder.hpp"
#include "world/TerrainGenerator.hpp" // ЭТО ОБЯЗАТЕЛЬНО
#include <glad/glad.h> // ДОБАВЛЕНО: для инициализации OpenGL функций

#include <imgui.h>
#include <imgui-SFML.h>
#include "Collisions.hpp"
#include "content/AssetPaths.hpp"
#include "core/IsoMath.hpp"
#include "render/GameRenderer.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp> // Необходимо для pushGLStates/popGLStates
#include "engine/Log.hpp"
#include <cstdlib>
#include <ctime>

namespace bunker
{
    namespace
    {
        glm::vec3 gameplayToRenderPosition(const Vector3D& position, float heightOffset = 0.0f)
        {
            return {position.x, position.z + heightOffset, position.y};
        }
    }

    GameApplication::GameApplication()
        : m_Window(sf::VideoMode({Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT}), "Bunker Protocol ISO",
            sf::Style::Close | sf::Style::Titlebar)
    {
        sf::ContextSettings settings;
        settings.depthBits = 24;        
        settings.stencilBits = 8;        
        settings.antiAliasingLevel = 4;
        settings.majorVersion = 3;       
        settings.minorVersion = 3;

        // В SFML 3 вместо sf::Style::Close используется sf::State::Windowed
        m_Window.create(sf::VideoMode({Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT}), 
                        "Bunker Protocol 3D", 
                        sf::State::Windowed, 
                        settings);

        m_GameState.hostileAI = &m_HostileAI;
        m_ImGuiInitialized = ImGui::SFML::Init(m_Window);
        

        // Инициализация нового 3D пайплайна
        m_Renderer3D.initialize();
    }

    int GameApplication::run()
    {
        initialize();
        bunker::logInfo() << "[SYSTEM] Bunker Protocol ISO запущен." << std::endl;
        
        // --- ДОБАВЬ ЭТОТ ЛОГ ---
        int frameCount = 0;
        while (m_Window.isOpen() && m_GameState.isRunning)
        {
            runFrame();
            
            // --- ДОБАВЬ ЭТОТ ЛОГ ---
            frameCount++;
        }

        bunker::logInfo() << "[SYSTEM] Выход из цикла. Кадров: " << frameCount << std::endl;
        shutdown();
        return 0;
    }

    void GameApplication::initialize()
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        generateAndLoadContent();
        generateWorld();
        restoreSaveIfAvailable();
        loadFonts();
        spawnInitialVehicles();

        m_Audio.playVoiceEvent(VoiceEvent::PipBoyBoot);
    }

    void GameApplication::shutdown()
    {
        for (auto& mesh : m_Registry.meshes.getRawData())
        {
            MeshBuilder::releaseFromGPU(mesh);
        }
        SaveSystem::writeSave(1, m_GameState, m_Inventory, &m_Registry);
        if (m_ImGuiInitialized)
        {
            ImGui::SFML::Shutdown(m_Window);
            m_ImGuiInitialized = false;
        }
        bunker::logInfo() << "[SYSTEM] Автосохранение при выходе. До встречи, Пилот." << std::endl;
    }

    void GameApplication::generateAndLoadContent()
    {
        // Генерация текстур при первом запуске.
        m_TextureGenerator.generateAllOnFirstRun();

        // Загрузка модульных конфигов. Пути централизованы и совпадают с lowercase-asset layout.
        m_VehicleManager.scanAndLoadConfigs(AssetPaths::VehiclesDir);
        m_EnemySpawner.scanAndLoadConfigs(AssetPaths::EnemiesDir);
    }

    void GameApplication::generateWorld()
    {
        m_WorldSession.generateDefaultWorld(m_GameState, m_EnemySpawner);
        m_Advanced.initialize(m_GameState, m_Inventory);

        assignInitialHostileProfiles();
        m_TimeShift.initialize(m_GameState, m_EnemySpawner);

        // 1. ГЕНЕРАЦИЯ ЗЕМЛИ (Авто-генератор)
        // Настраиваем параметры рельефа
        TerrainParams p;
        p.size = 100.0f;        // Размер поля
        p.heightScale = 3.0f;   // Высота холмов
        p.frequency = 0.15f;    // "Плотность" холмов

        EntityID groundEntity = m_Registry.createEntity();
        TransformComponent groundTransform;
        groundTransform.position = glm::vec3(0.0f, -2.0f, 0.0f); // Опускаем землю вниз
        
        m_Registry.transforms.insert(groundEntity, groundTransform);
        m_Registry.meshes.insert(groundEntity, TerrainGenerator::generateAutoTerrain(p));

        // 2. ОСТАВЛЯЕМ КУБ ДЛЯ ТЕСТА (можно увидеть, что он стоит НА земле)
        EntityID testCube = m_Registry.createEntity();
        TransformComponent transform;
        transform.position = glm::vec3(0.0f, 0.0f, 0.0f); 
        transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

        m_Registry.transforms.insert(testCube, transform);
        m_Registry.meshes.insert(testCube, MeshBuilder::createTestCube());
        createGameplayAnchors();

        // ВАЖНО: убедись, что твои методы renderGameplayFrame, processEdgeHotkeys и т.д. 
        // на месте. Я просто не стал дублировать их огромный текст для краткости.      
        // Можно заспавнить еще 1000 кубов циклом, и рендер даже не вспотеет, 
        // потому что данные лежат в памяти последовательно!
    }

    void GameApplication::createGameplayAnchors()
    {
        m_PlayerAnchorEntity = m_Registry.createEntity();
        TransformComponent playerTransform;
        playerTransform.scale = glm::vec3(0.45f, 0.9f, 0.45f);
        m_Registry.transforms.insert(m_PlayerAnchorEntity, playerTransform);
        m_Registry.meshes.insert(m_PlayerAnchorEntity, MeshBuilder::createTestCube());

        m_TitanAnchorEntity = m_Registry.createEntity();
        TransformComponent titanTransform;
        titanTransform.scale = glm::vec3(1.4f, 1.0f, 1.4f);
        m_Registry.transforms.insert(m_TitanAnchorEntity, titanTransform);
        m_Registry.meshes.insert(m_TitanAnchorEntity, MeshBuilder::createTestCube());

        syncGameplayAnchorsToECS();
    }

    void GameApplication::syncGameplayAnchorsToECS()
    {
        if (auto* playerTransform = m_Registry.transforms.get(m_PlayerAnchorEntity))
        {
            playerTransform->position = gameplayToRenderPosition(m_GameState.playerPos, 0.35f);
        }

        if (auto* titanTransform = m_Registry.transforms.get(m_TitanAnchorEntity))
        {
            titanTransform->position = gameplayToRenderPosition(m_GameState.titan.position, 0.55f);
        }
    }

    void GameApplication::assignInitialHostileProfiles()
    {
        // Старые Enemy из EnemySpawner получают полные профили из GMyGameDoNotTouch.
        for (std::size_t i = 0; i < m_GameState.enemies.size(); ++i)
        {
            HostileKind kind = HostileKind::VerminRush;
            if (i % 4 == 1)
                kind = HostileKind::GhoulRush;
            if (i % 4 == 2)
                kind = HostileKind::HumanTactical;
            if (i % 4 == 3)
            {
                kind = HostileKind::RobotControl;
                if (i == 3)
                {
                    m_BossAI.registerBoss(static_cast<int>(i), BossArchetype::RoySwarmPrime, {15.0f, 15.0f, 0.0f});
                }
            }
            m_HostileAI.assignKind(m_GameState, i, kind);
        }
    }

    void GameApplication::restoreSaveIfAvailable()
    {
        if (SaveSystem::saveExists(1))
        {
            SaveSystem::readSave(1, m_GameState, m_Inventory, &m_Registry);
            syncGameplayAnchorsToECS();
            bunker::logInfo() << "[SYSTEM] Сохранение восстановлено." << std::endl;
        }
    }

    void GameApplication::loadFonts()
    {
        m_FontLoaded = m_GlobalFont.openFromFile(AssetPaths::DefaultFont);
        m_Hud.loadFont(AssetPaths::DefaultFont);
        if (m_FontLoaded)
        {
            m_TerminalUI.loadFont(m_GlobalFont);
        }
        {
            m_MapScreen.loadFont(m_GlobalFont);
        }
    }

    void GameApplication::spawnInitialVehicles()
    {
        m_VehicleManager.spawnVehicle("steamcar", {3.0f, 3.0f, 0.0f});
        m_VehicleManager.spawnVehicle("motorcycle", {16.0f, 3.0f, 0.0f});
    }

    void GameApplication::runFrame()
    {
        float dt = clampDeltaTime(m_Clock.restart().asSeconds());
        m_GameState.deltaTime = dt;

        // InputManager единственный забирает SFML events; дополнительные хоткеи ниже обрабатываются edge-check'ом.
        InputSnapshot input = m_InputManager.capture(m_Window, m_ImGuiInitialized);
        if (m_ImGuiInitialized)
        {
            ImGui::SFML::Update(m_Window, sf::seconds(dt));
        }

        processEdgeHotkeys(input);

        if (input.quit)
        {
            m_GameState.isRunning = false;
        }

        // Терминал открыт — пауза gameplay
        if (m_TerminalUI.isOpen())
        {
            m_TerminalUI.handleWindowEvents(m_Window, m_GameState);
            m_TerminalUI.update(dt);
            
            // УБЕРИ clear, render и display отсюда.
            // Просто вызови рендер кадра. Логика на паузе, а мир нарисуется на фоне.
            renderGameplayFrame(); 
            return;
        }

        // Карта открыта — пауза gameplay, но карта продолжает получать pan-update.
        if (m_MapScreen.isOpen())
        {
            renderMapFrame(dt);
            return;
        }

        updateGameplayFrame(input, dt);
        renderGameplayFrame();
    }

    void GameApplication::processEdgeHotkeys(const InputSnapshot& input)
    {
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I), m_EdgeKeys.pipTabInv))
        {
            m_PipPad.toggleTab(m_GameState, 0);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J), m_EdgeKeys.pipTabMap))
        {
            m_PipPad.toggleTab(m_GameState, 1);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::O), m_EdgeKeys.pipTabTapes))
        {
            m_PipPad.toggleTab(m_GameState, 2);
        }

        // Доп. клавиши, которые раньше терялись из-за двойного pollEvent.
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M), m_EdgeKeys.map))
        {
            m_MapScreen.toggle();
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T), m_EdgeKeys.timeShift))
        {
            m_TimeShift.tryShift(m_GameState);
        }

        // Advanced hotkeys. Всё edge-triggered, чтобы не срабатывало 60 раз/сек.
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H), m_EdgeKeys.heal))
        {
            m_Advanced.survival.useStim(m_GameState, m_Inventory);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y), m_EdgeKeys.ration))
        {
            m_Advanced.survival.eatRation(m_GameState, m_Inventory, RationKind::Stamina);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R), m_EdgeKeys.reload))
        {
            switch (m_GameState.playerMode)
            {
            case UnitMode::Scout:
                m_Advanced.survival.startReload(m_Inventory);
                break;
            case UnitMode::Titan:
                m_TitanAI.cycleCockpitFireMode();
                break;
            default:
                break;
            }
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G), m_EdgeKeys.seatSwap) &&
            m_GameState.playerMode == UnitMode::Titan)
        {
            m_Advanced.tankUtility.swapSeat();
            bunker::logInfo() << "[COCKPIT] Место в кабине изменено → "
                            << (m_Advanced.tankUtility.runtime().seat == TankSeat::Driver ? "Водитель" : "Стрелок")
                            << std::endl;
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C), m_EdgeKeys.campCycle))
        {
            m_Advanced.camp.cycleType();
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::U), m_EdgeKeys.useUtility) &&
            m_GameState.playerMode == UnitMode::Titan)
        {
            m_Advanced.tankUtility.useUtility(m_GameState);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L), m_EdgeKeys.tape))
        {
            bunker::logInfo() << m_Advanced.radio.playNextUnplayed() << std::endl;
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F7), m_EdgeKeys.toolMode))
        {
            m_Advanced.toolgun.cycleMode();
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F8), m_EdgeKeys.toolUse) &&
            m_Inventory.hasItem(777))
        {
            m_Advanced.toolgun.apply(m_GameState, m_GameState.mouseWorldPos, m_Advanced.prefabs);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F2), m_EdgeKeys.undo))
        {
            m_Advanced.toolgun.undo(m_GameState);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F3), m_EdgeKeys.redo))
        {
            m_Advanced.toolgun.redo(m_GameState);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F6), m_EdgeKeys.delivery))
        {
            m_Advanced.lanline.requestDelivery("ammo", m_GameState.playerPos);
        }
        if (consumeEdge(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F4), m_EdgeKeys.wireframe))
        {
            m_RenderWireframe = !m_RenderWireframe;
            m_Renderer3D.setWireframeEnabled(m_RenderWireframe);
        }

        // B/toggleCamp остаётся в InputSnapshot и обрабатывается AdvancedMechanics::update.
        (void)input;
    }

    void GameApplication::renderMapFrame(float dt)
    {
        m_MapScreen.updatePan(dt);

        m_Window.clear(sf::Color(20, 20, 22));
        m_Window.setView(m_Window.getDefaultView());
        m_MapScreen.render(m_Window, m_GameState);
        m_Window.display();
    }

    void GameApplication::updateGameplayFrame(const InputSnapshot& input, float dt)
    {
        updateMouseWorldPosition(input);
        processSaveLoadInput(input);
        processModeSwitchInput(input);
        processPilotClassInput(input);
        updateGameplaySystems(input, dt);
    }

    void GameApplication::updateMouseWorldPosition(const InputSnapshot& input)
    {
        const sf::Vector2f screenPos = m_Window.mapPixelToCoords(input.mousePixelPos);
        m_GameState.mouseWorldPos = IsoMath::screenToWorld3D(screenPos.x, screenPos.y);
        m_GameState.isAiming = input.isAiming;
    }

    void GameApplication::processSaveLoadInput(const InputSnapshot& input)
    {
        if (input.saveGame)
        {
            SaveSystem::writeSave(1, m_GameState, m_Inventory, &m_Registry);
        }
        if (input.loadGame)
        {
            SaveSystem::readSave(1, m_GameState, m_Inventory, &m_Registry);
            syncGameplayAnchorsToECS();
        }
    }

    void GameApplication::processModeSwitchInput(const InputSnapshot& input)
    {
        if (!input.switchMode)
            return;

        if (m_GameState.playerMode == UnitMode::Scout)
        {
            if (m_TitanAI.tryMount(m_GameState))
            {
                m_Tactics.enterVehicle();
            }
        }
        else
        {
            m_TitanAI.dismount(m_GameState);
            m_Tactics.exitVehicle();
        }
    }

    void GameApplication::processPilotClassInput(const InputSnapshot& input)
    {
        if (m_GameState.playerMode != UnitMode::Scout || m_VehicleManager.isPlayerInVehicle())
        {
            return;
        }

        PilotClass classes[] = {PilotClass::Grapple,   PilotClass::Cloak, PilotClass::Stim,      PilotClass::PhaseShift,
                                PilotClass::HoloPilot, PilotClass::AWall, PilotClass::PulseBlade};

        for (int i = 0; i < 7; ++i)
        {
            if (input.selectClass[i])
            {
                m_Tactics.changePilotClass(classes[i]);
            }
        }
    }

    void GameApplication::updateGameplaySystems(const InputSnapshot& input, float dt)
    {
        m_TimeShift.update(dt);

        updateMovementAndInteraction(input, dt);
        updateTactics(input, dt);

        // Танк БТ-7274 всегда обновляется: и автономный, и пилотируемый.
        m_TitanAI.update(m_GameState, input, dt);

        updateCombat(input, dt);
        updateWorldSystems(input, dt);
        syncGameplayAnchorsToECS();

        m_Camera.update(dt, m_Window);
    }

    void GameApplication::updateMovementAndInteraction(const InputSnapshot& input, float dt)
    {
        if (m_VehicleManager.isPlayerInVehicle())
        {
            m_VehicleManager.update(m_GameState, input, dt);
            return;
        }

        if (m_GameState.playerMode != UnitMode::Scout)
        {
            return;
        }

        // Пешком.
        m_PlayerController.update(m_GameState, input, dt);

        // Посадка в транспорт, подключение к терминалу или подбор лута (E).
        if (input.interact)
        {
            if (!m_TerminalUI.tryInteractTerminal(m_GameState))
            {
                if (!m_VehicleManager.mountNearest(m_GameState))
                {
                    m_WorldSession.interactWithContainers(m_GameState, m_Inventory);
                }
            }
        }
    }

    void GameApplication::updateTactics(const InputSnapshot& input, float dt)
    {
        m_Tactics.updateCooldowns(m_GameState, dt);

        if (input.activateTactical && m_GameState.playerMode == UnitMode::Scout &&
            !m_VehicleManager.isPlayerInVehicle())
        {
            m_Tactics.activateTactical(m_GameState, m_GameState.mouseWorldPos);
        }

        m_Tactics.processGrapplePhysics(m_GameState, dt);
    }

    void GameApplication::updateCombat(const InputSnapshot& input, float dt)
    {
        if (input.isShooting && m_GameState.fireCooldown <= 0.0f)
        {
            switch (m_GameState.playerMode)
            {
            case UnitMode::Scout:
                if (m_VehicleManager.isPlayerInVehicle())
                    break;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F10))
                {
                    m_BulletSystem.fireDebugGunChainLightning(m_GameState);
                }
                else
                {
                    if (m_Advanced.survival.consumeRound())
                    {
                        m_BulletSystem.fireScoutWeapon(m_GameState, m_GameState.isAiming);
                    }
                    else
                    {
                        m_Advanced.survival.startReload(m_Inventory);
                    }
                }
                break;
            case UnitMode::Titan:
            {
                float heat = m_GameState.titan.currentWeapon == TankWeaponMode::Cannon ? 24.0f : 7.0f;
                if (m_Advanced.tankUtility.registerCannonShot(m_GameState, heat))
                {
                    m_TitanAI.fireFromCockpit(m_GameState);
                }
                break;
            }
            default:
                break;
            }
        }

        // Ракеты Титана (СКМ).
        if (input.isMissile && m_GameState.playerMode == UnitMode::Titan)
        {
            m_BulletSystem.fireTitanMissiles(m_GameState);
        }

        // Автострельба Танка (автономный режим).
        m_TitanAI.autoFire(m_GameState);

        // Баллистика.
        m_BulletSystem.update(m_GameState, dt);
    }

    void GameApplication::updateWorldSystems(const InputSnapshot& input, float dt)
    {
        // Враги: спавнер отвечает за волны, поведение врагов ведёт HostileAISystem.
        m_EnemySpawner.updateWaveSpawning(m_GameState, dt);
        m_HostileAI.update(m_GameState, dt);
        m_BossAI.updateBosses(m_GameState, dt);
        PerceptionSystem::updateStimuli(dt);

        Collisions::resolveAllCollisions(m_GameState);

        // Мир: эрозия, осада, Pip-Pad.
        m_WorldSession.update(m_GameState, dt);
        m_PipPad.update(m_GameState, dt);

        m_Audio.update(dt);
        m_ThermalLoad.updateThermodynamics(m_GameState, dt);
        StoryEventManager::evaluateZoneTriggers(m_GameState, m_StoryFlags);

        // Broadphase регистрация сетки
        m_SpatialGrid.clearBuckets();
        m_SpatialGrid.registerEntity(0, m_GameState.playerPos);
        m_SpatialGrid.registerEntity(1, m_GameState.titan.position);
        for (std::size_t i = 0; i < m_GameState.enemies.size(); ++i)
        {
            if (m_GameState.enemies[i].isAlive)
            {
                m_SpatialGrid.registerEntity(i + 2, m_GameState.enemies[i].position);
            }
        }

        // Advanced переносы из двух старых репозиториев.
        m_Advanced.update(m_GameState, m_Inventory, input, dt);

        // C.A.M.P. place: B включает режим, ЛКМ ставит объект если не стреляем по врагу.
        if (m_Advanced.camp.enabled() && input.isShooting)
        {
            m_Advanced.camp.place(m_GameState, m_Inventory, m_Advanced.skills.buildCostMultiplier());
        }
    }

    // 2. ИЗМЕНЕННЫЙ МЕТОД ОТРИСОВКИ
    void GameApplication::renderGameplayFrame()
    {
        // 1. Очистка экрана перед отрисовкой
        glClearColor(0.06f, 0.07f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // =========================================================
        // СЛОЙ 1: 3D МИР (OpenGL / ECS)
        // =========================================================
        
        // Включаем тест глубины для правильного отображения 3D объектов
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        // Рендерим 3D-сцену один раз
        m_Renderer3D.renderScene(m_Registry, m_Camera);
        
        // =========================================================
        // ПЕРЕХОД К 2D (SFML / ImGui)
        // =========================================================
        
        // Отключаем 3D-фичи, чтобы они не влияли на плоский интерфейс
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        // Сохраняем стейт 3D контекста OpenGL, чтобы 2D-команды SFML его не сломали
        m_Window.pushGLStates();

        // Устанавливаем стандартную камеру (экранное пространство 1:1)
        m_Window.setView(m_Window.getDefaultView());
        
        GameRenderer::renderFloor(m_Window, m_GameState, m_TimeShift);
        GameRenderer::renderEntities(m_Window, m_GameState, m_TimeShift, m_HostileAI);
        GameRenderer::renderAdvancedWorld(m_Window, m_Advanced);

        m_Hud.render(m_Window, m_GameState, m_PlayerController, m_Tactics, m_TitanAI, m_VehicleManager, m_Inventory);
        
        if (m_TimeShift.isInitialized()) {
            m_TimeShift.renderHUD(m_Window, m_FontLoaded ? &m_GlobalFont : nullptr);
            m_TimeShift.renderTransitionEffect(m_Window);
        }

        GameRenderer::renderAdvancedHUD(m_Window, m_Advanced, m_FontLoaded ? &m_GlobalFont : nullptr);
        m_Audio.renderSubtitlesHUD(m_Window, m_FontLoaded ? &m_GlobalFont : nullptr);
        m_PipPad.renderTablet(m_Window, m_GameState, m_Inventory, m_Advanced, m_FontLoaded ? &m_GlobalFont : nullptr);

        if (m_TerminalUI.isOpen()) {
            m_TerminalUI.render(m_Window, m_GameState);
        }

        if (m_ImGuiInitialized) {
            ImGui::SFML::Render(m_Window);
        }

        // Восстанавливаем аппаратный контекст OpenGL после 2D
        m_Window.popGLStates();

        // Финальный вывод кадра на экран
        m_Window.display(); 
    }// <--- ЭТА СКОБКА ЗАКРЫВАЕТ ФУНКЦИЮ renderGameplayFrame

    bool GameApplication::consumeEdge(bool isPressedNow, bool& wasPressedBefore)
    {
        bool edge = isPressedNow && !wasPressedBefore;
        wasPressedBefore = isPressedNow;
        return edge;
    }

    float GameApplication::clampDeltaTime(float dt)
    {
        return (dt > 0.1f) ? 0.1f : dt;
    }

} // namespace bunker
