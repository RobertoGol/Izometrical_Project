cmake_minimum_required(VERSION 3.16)
project(BunkerProtocolISO VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)

set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/source/SFML-3.1.0" CACHE PATH "Bundled SFML root")
set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML" CACHE PATH "SFML CMake package directory")

find_package(SFML 3.1 CONFIG REQUIRED COMPONENTS Audio Graphics Window System)
find_package(OpenGL REQUIRED)

# --- Загрузка GLM ---
add_library(glm INTERFACE)
set(LOCAL_GLM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.deps/glm-0.9.9.8")
if(EXISTS "${LOCAL_GLM_SOURCE_DIR}/glm/glm.hpp")
    target_include_directories(glm SYSTEM INTERFACE "${LOCAL_GLM_SOURCE_DIR}")
else()
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
    )
    FetchContent_GetProperties(glm)
    if(NOT glm_POPULATED)
        FetchContent_Populate(glm)
    endif()
    target_include_directories(glm SYSTEM INTERFACE "${glm_SOURCE_DIR}")
endif()
# --------------------

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/ImGui")

add_library(imgui_sfml_vendor STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
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
    src/glad.c
    src/ai/BossAISystem.cpp
    src/ai/EnemyArchetypeRegistry.cpp
    src/ai/EnemySpawner.cpp
    src/ai/HostileAISystem.cpp
    src/ai/HostileAISystemBehavior.cpp
    src/ai/HostileAISystemMovement.cpp
    src/ai/HostileAISystemProfiles.cpp
    src/ai/PerceptionSystem.cpp
    src/ai/TitanAI.cpp
    src/ai/TitanAICockpit.cpp
    src/ai/TitanAIMovement.cpp
    src/ai/TitanAISystems.cpp
    src/content/Workstations.cpp
    src/content/MeshBuilder.cpp
    src/DevMenu.cpp
    src/DevMenuConsole.cpp
    src/engine/AudioController.cpp
    src/engine/GameApplication.cpp
    src/engine/GameApplicationECSBridge.cpp
    src/engine/GameApplicationRender.cpp
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
    src/gameplay/AdvancedMechanicsCamp.cpp
    src/gameplay/AdvancedMechanicsLanline.cpp
    src/gameplay/AdvancedMechanicsObjModel.cpp
    src/gameplay/AdvancedMechanicsProgression.cpp
    src/gameplay/AdvancedMechanicsRadio.cpp
    src/gameplay/AdvancedMechanicsSurvival.cpp
    src/gameplay/AdvancedMechanicsTankUtility.cpp
    src/gameplay/AdvancedMechanicsServices.cpp
    src/gameplay/AdvancedMechanicsToolGun.cpp
    src/gameplay/AdvancedMechanicsWorld.cpp
    src/gameplay/AdvancedPilotMovement.cpp
    src/gameplay/BulletSystem.cpp
    src/gameplay/BulletSystemEffects.cpp
    src/gameplay/BulletSystemFire.cpp
    src/gameplay/BulletSystemRender.cpp
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
    src/render/Renderer3D.cpp
    src/render/ShaderManager.cpp
    src/ResourceManager.cpp
    src/ui/HUD.cpp
    src/ui/PipPadUI.cpp
    src/ui/TerminalUI.cpp
    src/ui/WorkstationUI.cpp
    src/vehicles/ThermalLoadSystem.cpp
    src/vehicles/VehicleInstance.cpp
    src/vehicles/VehicleManager.cpp
    src/vehicles/VehicleSeatController.cpp
    src/world/WeatherSystemDraw.cpp
    src/world/WeatherSystemEffects.cpp
    src/world/WeatherSystemLogic.cpp
    src/world/WeatherSystemRender.cpp
    src/WorkstationDatabase.cpp
    src/WorkstationManager.cpp
    src/Workstations.cpp
    src/world/CampPlacementValidator.cpp
    src/world/DestructibleEnvironment.cpp
    src/world/MapScreen.cpp
    src/world/StoryEventManager.cpp
    src/world/WeatherSystem.cpp
    src/world/WorldSession.cpp
    src/world/TerrainGenerator.cpp
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
    glm
    OpenGL::GL
    imgui_sfml_vendor
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

# Копирование ассетов и DLL
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    )
endif()

if(EXISTS "${SFML_ROOT}/bin")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SFML_ROOT}/bin"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
endif()

if(BUILD_TESTING)
    add_test(
        NAME smoke_assets
        COMMAND ${CMAKE_COMMAND}
            -DAPP_PATH=$<TARGET_FILE:${PROJECT_NAME}>
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SmokeTest.cmake"
    )
endif()
