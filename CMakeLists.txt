cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolISO VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(SOURCES
    src/main.cpp
    src/engine/GameApplication.cpp
    src/engine/AudioController.cpp
    src/engine/ProceduralSoundEffects.cpp
    src/engine/InputManager.cpp
    src/engine/TimeShift.cpp
    src/world/WorldSession.cpp
    src/world/MapScreen.cpp
    src/world/WeatherSystem.cpp
    src/world/CampPlacementValidator.cpp
    src/world/StoryEventManager.cpp
    src/world/DestructibleEnvironment.cpp
    src/vehicles/VehicleManager.cpp
    src/vehicles/VehicleSeatController.cpp
    src/vehicles/ThermalLoadSystem.cpp
    src/gameplay/Tactics.cpp
    src/gameplay/BulletSystem.cpp
    src/gameplay/AdvancedMechanics.cpp
    src/gameplay/DamageSystem.cpp
    src/gameplay/SpatialGrid.cpp
    src/gameplay/StatefulActionsController.cpp
    src/gameplay/AdvancedPilotMovement.cpp
    src/ai/EnemySpawner.cpp
    src/ai/EnemyArchetypeRegistry.cpp
    src/ai/HostileAISystem.cpp
    src/ai/PerceptionSystem.cpp
    src/ai/TitanAI.cpp
    src/persistence/Inventory.cpp
    src/persistence/Progression.cpp
    src/persistence/CharacterManager.cpp
    src/persistence/StoryFlagRegistry.cpp
    src/persistence/ActiveBuffsController.cpp
    src/persistence/SaveSystem.cpp
    src/ui/HUD.cpp
    src/ui/TerminalUI.cpp
    src/ui/PipPadUI.cpp
    src/render/GameRenderer.cpp
    src/entities/PlayerController.cpp
    src/entities/Camera.cpp
    src/entities/Collisions.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/include/content
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/include/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gameplay
    ${CMAKE_CURRENT_SOURCE_DIR}/include/persistence
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vehicles
    ${CMAKE_CURRENT_SOURCE_DIR}/include/world
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets to build directory..."
    )
endif()