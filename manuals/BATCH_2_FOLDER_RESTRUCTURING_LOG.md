Batch 2: Core Folder Restructuring & Baseline Cleanup Log
Date: 2026-06-26

Executive Summary
This batch continues the core stabilization efforts started in Batch 1, following the architectural guidelines in TARGET_ARCHITECTURE.md and INTEGRATION_PHASES.md (Phase 1 & 2). The primary objective was to organize the repository files into clean domain directories without deleting any existing mechanics, classes, or legacy logic.

1. Filesystem & Asset Path Case Normalization
Status: COMPLETED
Executed actual git mv operations to normalize asset directory names on case-sensitive filesystems (Linux):
assets/Enemies/ → assets/enemies/
assets/Vehicles/ → assets/vehicles/
Verified compatibility with include/content/AssetPaths.hpp runtime constants (AssetPaths::VehiclesDir, AssetPaths::EnemiesDir).
2. Header Reorganization (include/)
Status: COMPLETED
Migrated 24 flat header files from the root of include/ into structured domain subdirectories matching TARGET_ARCHITECTURE.md:
include/core/: Constants.hpp, Types.hpp, IsoMath.hpp
include/engine/: InputManager.hpp, TimeShift.hpp (alongside existing GameApplication.hpp)
include/world/: WorldSession.hpp, WeatherSystem.hpp, MapScreen.hpp
include/entities/: PlayerController.hpp, Camera.hpp, Collisions.hpp
include/gameplay/: BulletSystem.hpp, Tactics.hpp, AdvancedMechanics.hpp, GameState.hpp
include/ai/: HostileAISystem.hpp, EnemySpawner.hpp, TitanAI.hpp
include/vehicles/: VehicleManager.hpp
include/ui/: HUD.hpp
include/persistence/: SaveSystem.hpp, Progression.hpp, Inventory.hpp
include/content/: TextureGenerator.hpp (alongside existing AssetPaths.hpp)
include/render/: (Existing GameRenderer.hpp)
3. Thin Bootstrap Cleanup (src/main.cpp)
Status: COMPLETED
Removed residual legacy code left at the bottom of src/main.cpp during Batch 1.
src/main.cpp now acts strictly as a thin bootstrap entry point:
C++

#include "engine/GameApplication.hpp"

int main() {
    bunker::GameApplication app;
    return app.run();
}
All application lifecycle, game loop updates, and rendering logic remain safely encapsulated within src/engine/GameApplication.cpp and src/render/GameRenderer.cpp. Zero mechanics or features were deleted.
4. Build System Harmonization (CMakeLists.txt)
Status: COMPLETED
Updated target_include_directories in CMakeLists.txt to include all new domain subdirectories under include/.
This ensures 100% backward compatibility for all existing unqualified #include "Header.hpp" statements while enabling clean modern C++ relative imports (#include "core/Types.hpp").
5. Verification & Next Steps
Git State: Clean staged renames via git mv ensuring full preservation of file history.
Next Phase: Proceed to Phase 2 deep mechanics integration (transferring remaining hostile AI profiles, survival thresholds, and vehicle modules from GMyGameDoNotTouch and Video_Game_Izom).
