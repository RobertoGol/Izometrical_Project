Batch 1 Core Cleanup Log
Date: 2026-06-26

Scope
Implemented the first core stabilization cleanup from MASTER_MIGRATION_PLAN.md:

normalize asset folder case;
split main.cpp god-file into application/runtime and renderer modules;
keep main.cpp as a thin bootstrap;
centralize the first hardcoded runtime asset paths.
Changes
Asset paths
Status: DONE

Renamed assets/Enemies/ to assets/enemies/.
Renamed assets/Vehicles/ to assets/vehicles/.
Added include/content/AssetPaths.hpp with lowercase runtime constants:
AssetPaths::VehiclesDir
AssetPaths::EnemiesDir
AssetPaths::DefaultFont
Verified that src/, include/, and CMakeLists.txt no longer reference assets/Enemies or assets/Vehicles.
Runtime split
Status: DONE for the first pass; compile/run validation is still pending in an SFML-enabled environment.

Replaced src/main.cpp with a thin bootstrap:
constructs bunker::GameApplication;
calls run().
Added include/engine/GameApplication.hpp.
Added src/engine/GameApplication.cpp.
Moved application lifecycle, system ownership, initialization, frame loop, input hotkeys, update ordering, map pause flow, save/load dispatch, and shutdown autosave out of main.cpp.
Renderer split
Status: DONE for the first pass.

Added include/render/GameRenderer.hpp.
Added src/render/GameRenderer.cpp.
Moved these former main.cpp render helpers into GameRenderer:
floor rendering;
entity Z-sorted rendering;
advanced world rendering;
advanced HUD rendering.
Added explicit <sstream> include for advanced HUD text composition.
Build metadata
Status: DONE

Updated CMakeLists.txt source list:

src/main.cpp
src/engine/GameApplication.cpp
src/render/GameRenderer.cpp
src/entities/PlayerController.cpp
Validation performed
Checked source file presence against CMakeLists.txt.
Checked braces/parentheses balance on newly written files.
Checked that code paths do not reference old mixed-case asset directories.
Attempted local syntax/build check, but sandbox validation is blocked because this environment has neither CMake nor SFML headers installed.
Validation blocked
Status: BLOCKED

The command below cannot complete in the current sandbox:

Bash

g++ -std=c++17 -Iinclude -fsyntax-only \
  src/main.cpp \
  src/engine/GameApplication.cpp \
  src/render/GameRenderer.cpp \
  src/entities/PlayerController.cpp
Reason:

text

fatal error: SFML/Graphics.hpp: No such file or directory
A full compile/run smoke test should be performed on a machine with CMake and SFML 2.5+ installed.

Next recommended step
Continue Batch 1 with:

compile/run smoke test in a proper SFML environment;
include/TitanAI.hpp audit;
split or document the next largest ownership issue before adding more mechanics.
