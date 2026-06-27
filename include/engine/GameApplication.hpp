#pragma once

#include <SFML/Graphics.hpp>

#include "AdvancedMechanics.hpp"
#include "BulletSystem.hpp"
#include "Camera.hpp"
#include "EnemySpawner.hpp"
#include "GameState.hpp"
#include "HUD.hpp"
#include "HostileAISystem.hpp"
#include "InputManager.hpp"
#include "Inventory.hpp"
#include "MapScreen.hpp"
#include "PlayerController.hpp"
#include "Progression.hpp"
#include "SaveSystem.hpp"
#include "Tactics.hpp"
#include "TextureGenerator.hpp"
#include "TimeShift.hpp"
#include "TitanAI.hpp"
#include "VehicleManager.hpp"
#include "WorldSession.hpp"

namespace bunker {

// Engine Runtime boundary: owns the application lifecycle, update order and
// dispatch to gameplay/render subsystems. main.cpp intentionally stays thin.
class GameApplication {
public:
    GameApplication();

    int run();

private:
    struct EdgeKeyState {
        bool map = false;
        bool timeShift = false;
        bool heal = false;
        bool ration = false;
        bool reload = false;
        bool campCycle = false;
        bool useUtility = false;
        bool tape = false;
        bool toolMode = false;
        bool toolUse = false;
        bool undo = false;
        bool redo = false;
        bool delivery = false;
        bool seatSwap = false;
    };

    sf::RenderWindow m_Window;

    GameState        m_GameState;
    InputManager     m_InputManager;
    Camera           m_Camera;
    PlayerController m_PlayerController;
    TacticsManager   m_Tactics;
    BulletSystem     m_BulletSystem;
    TitanAI          m_TitanAI;
    VehicleManager   m_VehicleManager;
    EnemySpawner     m_EnemySpawner;
    WorldSession     m_WorldSession;
    PlayerInventory  m_Inventory;
    HUD              m_Hud;
    MapScreen        m_MapScreen;
    TimeShift        m_TimeShift;
    TextureGenerator m_TextureGenerator;
    HostileAISystem  m_HostileAI;
    AdvancedMechanics m_Advanced;

    sf::Clock m_Clock;
    sf::Font  m_GlobalFont;
    bool      m_FontLoaded = false;

    EdgeKeyState m_EdgeKeys;

    void initialize();
    void shutdown();

    void generateAndLoadContent();
    void generateWorld();
    void assignInitialHostileProfiles();
    void restoreSaveIfAvailable();
    void loadFonts();
    void spawnInitialVehicles();

    void runFrame();
    void processEdgeHotkeys(const InputSnapshot& input);
    void renderMapFrame(float dt);
    void updateGameplayFrame(const InputSnapshot& input, float dt);
    void renderGameplayFrame();

    void updateMouseWorldPosition(const InputSnapshot& input);
    void processSaveLoadInput(const InputSnapshot& input);
    void processModeSwitchInput(const InputSnapshot& input);
    void processPilotClassInput(const InputSnapshot& input);

    void updateGameplaySystems(const InputSnapshot& input, float dt);
    void updateMovementAndInteraction(const InputSnapshot& input, float dt);
    void updateTactics(const InputSnapshot& input, float dt);
    void updateCombat(const InputSnapshot& input, float dt);
    void updateWorldSystems(const InputSnapshot& input, float dt);

    static bool consumeEdge(bool isPressedNow, bool& wasPressedBefore);
    static float clampDeltaTime(float dt);
};

}  // namespace bunker
