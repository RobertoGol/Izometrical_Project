cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolISO VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/source/SFML-3.1.0" CACHE PATH "Bundled SFML root")
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML" CACHE PATH "SFML CMake package directory")

find_package(SFML 3.1 CONFIG REQUIRED COMPONENTS Audio Graphics Window System)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/ImGui")

add_library(imgui_sfml_vendor STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui-SFML.cpp"
)

target_include_directories(imgui_sfml_vendor PUBLIC
    "${IMGUI_DIR}"
)

target_link_libraries(imgui_sfml_vendor PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenGL::GL
)

add_library(game_core STATIC
    src/ai/BossAISystem.cpp
    src/ai/EnemyArchetypeRegistry.cpp
    src/ai/EnemySpawner.cpp
    src/ai/HostileAISystem.cpp
    src/ai/PerceptionSystem.cpp
    src/ai/TitanAI.cpp
    src/ai/TitanAICockpit.cpp
    src/ai/TitanAIMovement.cpp
    src/ai/TitanAISystems.cpp
    src/content/Workstations.cpp
    src/DevMenu.cpp
    src/engine/AudioController.cpp
    src/engine/GameApplication.cpp
    src/engine/InputManager.cpp
    src/engine/ProceduralSoundEffects.cpp
    src/engine/ProceduralSoundEffectsEnvironment.cpp
    src/engine/ProceduralSoundEffectsMechanical.cpp
    src/engine/ProceduralSoundEffectsPilot.cpp
    src/engine/ProceduralSoundEffectsWeapons.cpp
    src/engine/TimeShift.cpp
    src/entities/Camera.cpp
    src/entities/Collisions.cpp
    src/entities/PlayerController.cpp
    src/gameplay/AdvancedMechanics.cpp
    src/gameplay/AdvancedMechanicsSurvival.cpp
    src/gameplay/AdvancedMechanicsWorld.cpp
    src/gameplay/AdvancedMechanicsServices.cpp
    src/gameplay/AdvancedPilotMovement.cpp
    src/gameplay/BulletSystem.cpp
    src/gameplay/DamageSystem.cpp
    src/gameplay/SpatialGrid.cpp
    src/gameplay/StatefulActionsController.cpp
    src/gameplay/Tactics.cpp
    src/persistence/ActiveBuffsController.cpp
    src/persistence/CharacterManager.cpp
    src/persistence/Inventory.cpp
    src/persistence/ModularEquipmentSystem.cpp
    src/persistence/Progression.cpp
    src/persistence/SaveSystem.cpp
    src/persistence/StoryFlagRegistry.cpp
    src/RecipeDatabase.cpp
    src/render/GameRenderer.cpp
    src/render/GameRendererAdvancedHUD.cpp
    src/render/GameRendererAdvancedWorld.cpp
    src/render/GameRendererEntities.cpp
    src/ResourceManager.cpp
    src/TextureGenerator.cpp
    src/ui/HUD.cpp
    src/ui/PipPadUI.cpp
    src/ui/TerminalUI.cpp
    src/ui/WorkstationUI.cpp
    src/vehicles/ThermalLoadSystem.cpp
    src/vehicles/VehicleManager.cpp
    src/vehicles/VehicleSeatController.cpp
    src/WorkstationDatabase.cpp
    src/WorkstationManager.cpp
    src/Workstations.cpp
    src/world/CampPlacementValidator.cpp
    src/world/DestructibleEnvironment.cpp
    src/world/MapScreen.cpp
    src/world/StoryEventManager.cpp
    src/world/WeatherSystem.cpp
    src/world/WorldSession.cpp
    src/WorldWorkstation.cpp
)

target_include_directories(game_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ai"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/content"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/engine"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/entities"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gameplay"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/persistence"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/render"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/vehicles"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/world"
)

target_link_libraries(game_core PUBLIC
    SFML::Audio
    SFML::Graphics
    SFML::Window
    SFML::System
)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    game_core
    imgui_sfml_vendor
)

if(MSVC)
    target_compile_options(game_core PRIVATE /W4 /permissive-)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(game_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets to build directory..."
    )
endif()

if(EXISTS "${SFML_ROOT}/bin")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SFML_ROOT}/bin"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying SFML runtime DLLs to build directory..."
    )
endif()
