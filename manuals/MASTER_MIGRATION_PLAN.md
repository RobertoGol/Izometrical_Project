# MASTER_MIGRATION_PLAN

## Project Goal

Final target: transform `Izometrical_Project` into the single, stable, fully playable handwritten-engine game that merges and improves mechanics from:

- `GMyGameDoNotTouch`
- `Video_Game_Izom`

No Unity, no Godot, no external full engine migration.  
Only our own codebase and our own runtime architecture.

This document is the master migration map.  
Its purpose is to prevent lost mechanics, prevent partial merges, and guarantee that all useful systems from source repositories are either:

1. fully integrated,
2. deliberately redesigned,
3. or explicitly rejected with reason.

---

# 1. Source Repositories

## 1.1 Primary target repository
- `Izometrical_Project`

This is the final production repository.

## 1.2 Source repository A
- `GMyGameDoNotTouch`

This repository contains:
- large gameplay runtime logic
- combat and enemy behavior ideas
- stress/survival systems
- tank/titan interaction logic
- story/event scripting
- environment hazards
- utility gameplay systems
- a lot of buggy but valuable mechanics

## 1.3 Source repository B
- `Video_Game_Izom`

This repository contains:
- isometric gameplay base
- world interaction patterns
- likely build/camp/tool/editor systems
- utility architecture pieces
- map/gameplay support code
- additional mechanics that must be merged into target project

---

# 2. Non-Negotiable Rules

## 2.1 Final repository rule
All final gameplay-important mechanics must live in:
- `Izometrical_Project`

## 2.2 Quality rule
We do not copy bugs.  
We migrate intent, logic, content, and systems — but repair bad architecture where needed.

## 2.3 Engine rule
The project remains a handwritten engine/codebase.  
Allowed:
- our own renderer
- our own ECS-like or object architecture
- our own loaders
- our own AI/state systems

Not allowed:
- replacing project with Unity/Godot/Unreal equivalent workflow

## 2.4 Merge rule
Every imported mechanic must end in one of these states:

- `DONE`
- `PARTIAL`
- `BLOCKED`
- `REPLACED`
- `REJECTED`

No silent omission allowed.

---

# 3. Final Migration Philosophy

We are not doing a blind copy-paste.

We are building:

1. a stable core runtime,
2. then reintroducing mechanics in dependency order,
3. then connecting them through shared systems,
4. then polishing balance and content,
5. then removing dead legacy code.

The result must be:
- playable
- debuggable
- extensible
- understandable

---

# 4. High-Level Work Phases

## Phase 0 — Full Audit
Read everything important from all repos:
- branches
- commits
- markdown docs
- runtime files
- gameplay classes
- assets folder structure
- config files
- TODO comments
- old prototypes worth salvaging

Output:
- migration checklist
- mechanic inventory
- architecture diff
- asset dependency list

## Phase 1 — Core Stabilization
Before adding more mechanics:
- fix compile errors
- normalize folder layout
- normalize naming and include paths
- remove broken dependencies
- ensure main loop is stable
- ensure deterministic update ordering
- ensure save/load boundaries are clear

## Phase 2 — Core Gameplay Framework
Unify systems for:
- entity update
- movement
- collision
- combat
- damage
- inventory
- interaction
- AI perception
- events
- serialization

## Phase 3 — Critical Gameplay Recovery
Restore mechanics that directly affect "game exists":
- enemies
- player combat
- titan/tank systems
- health/healing/reload
- world hazards
- loot
- progression

## Phase 4 — World Systems
Restore:
- events
- terminals
- breakables
- weather
- map interactions
- dynamic world state

## Phase 5 — Advanced Systems
Restore or redesign:
- stress/second wind/soulline
- modular vehicles
- hangar systems
- skills
- editor features
- possible networking support if still intended

## Phase 6 — Content Completion
- enemy tuning
- world placement
- item tables
- tape/radio content
- story triggers
- progression pacing

## Phase 7 — Polish and Ship-Readiness
- bug fixing
- balancing
- perf work
- cleanup
- dead code removal
- documentation
- regression tests/checklists

---

# 5. Current Confirmed Important Missing Mechanics

Below is the current master list of major mechanics identified as missing or incompletely transferred.

---

## 5.1 From `GMyGameDoNotTouch`

### A. Enemy / Combat AI
1. HostileAI system
   - enemy archetypes:
     - VerminRush
     - GhoulRush
     - HumanTactical
     - RobotControl

2. Awareness / aggro logic
   - delayed detection
   - awareness buildup
   - attack state transition

3. Mechanical hostile localized damage
   - sensor damage
   - weapon damage
   - mobility damage

4. Shock wave effects
   - explosions disturb aim / motion / combat readability

### B. Survival / Combat Loop
5. Stress system
6. SecondWind system
7. SoulLine last-chance system
8. Heal action
9. Reload action
10. Rations with temporary buffs

### C. Titan / Vehicle / Heavy System
11. Bucket Rig
12. Ram Shield
13. Tow Coupler
14. Seat swapping
15. Tank thermal load / overheating
16. cockpit-based fire logic
17. mount / dismount flow
18. titan internal state management improvements

### D. World / Interaction
19. Weather system
   - EtherFog
   - AcidRain

20. Radio / Tape system
21. Terminal sync / pip-pad sync
22. Hangar system
23. Reactive breakables
24. Zone events / scripted progression

### E. Progression / Systems
25. Skill system
26. Loot generator by tiers
27. Profile / session management
28. Story route progression

### F. Tooling / Meta
29. World editor features
30. prefab library

### G. Optional / likely redesign-required
31. Lanline services / multiplayer-adjacent features

---

## 5.2 From `Video_Game_Izom`

### A. Build / Utility
1. C.A.M.P.-style building system
   - walls
   - turrets
   - boxes / utility placement

2. ToolGun / live editor interaction

### B. Runtime / Performance
3. SpatialGrid optimization for collisions and lookups

### C. Asset / Rendering
4. OBJ model loading support

### D. Networking
5. Winsock multiplayer support up to ~20 players

---

# 6. Priority Order of Migration

We should not migrate by "coolness".  
We migrate by dependency.

## Tier 1 — Must exist first
These are foundational:

- game loop stability
- entity lifecycle
- collision
- damage pipeline
- animation/state timing
- inventory core
- item use pipeline
- AI perception base
- event dispatch / trigger base
- save/load boundaries
- asset path normalization

## Tier 2 — Immediate gameplay completeness
These make the game genuinely playable:

- enemy archetypes
- aggro/awareness
- heal/reload
- loot
- weather hazards
- breakables
- stress system
- titan mount/dismount
- seat switching
- core tank combat

## Tier 3 — Progression and world depth
- terminals
- hangar
- zone events
- skill system
- rations
- tape/radio
- story route

## Tier 4 — Advanced / optional but important
- C.A.M.P. building
- toolgun
- spatial grid
- OBJ loading
- profile migration/versioning
- editor features

## Tier 5 — Only after single-player stability
- networking / lanline / multiplayer

---

# 7. First Architecture Targets in `Izometrical_Project`

These are the first file categories that must be audited and rewritten carefully if needed.

## 7.1 Runtime entry
- `main.cpp`

Must become:
- minimal bootstrap
- clean ownership
- no giant god-file logic
- delegates to systems/modules

## 7.2 Titan / vehicle logic
- `include/TitanAI.hpp`
- any titan cpp/hpp pair
- vehicle runtime modules

Must include:
- mount/dismount
- seat state
- cockpit fire
- thermal load
- module state
- stress-aware behavior if linked

## 7.3 Enemy AI module
Need dedicated files, likely:
- `EnemyAI.hpp/.cpp`
- `Perception.hpp/.cpp`
- `CombatAI.hpp/.cpp`

Instead of bloating `main.cpp`.

## 7.4 Player systems
Need separation for:
- player state
- health
- healing
- reload
- stress
- interaction
- inventory

## 7.5 World systems
Need dedicated modules for:
- weather
- breakables
- scripted zones
- terminals
- loot spawning

---

# 8. Recommended Target Module Layout

This is the preferred structure for the final repo.

```text
include/
  Core/
    Game.hpp
    Time.hpp
    Config.hpp
    Logger.hpp
    Events.hpp
  World/
    World.hpp
    TileMap.hpp
    SpatialGrid.hpp
    ZoneEvents.hpp
    WeatherSystem.hpp
    BreakableSystem.hpp
  Entities/
    Entity.hpp
    EntityManager.hpp
    Components.hpp
  Player/
    Player.hpp
    PlayerInventory.hpp
    PlayerCombat.hpp
    PlayerStress.hpp
    PlayerInteraction.hpp
  AI/
    EnemyAI.hpp
    HostileArchetypes.hpp
    PerceptionSystem.hpp
    TacticalLogic.hpp
  Vehicles/
    TitanAI.hpp
    TitanModules.hpp
    VehicleCombat.hpp
    VehicleThermal.hpp
    MountSystem.hpp
  Items/
    Item.hpp
    LootSystem.hpp
    Rations.hpp
    TapeSystem.hpp
  Progression/
    SkillSystem.hpp
    ProfileSystem.hpp
    StorySystem.hpp
  UI/
    PipPad.hpp
    HUD.hpp
    TerminalUI.hpp
  Tools/
    WorldEditor.hpp
    ToolGun.hpp
	
	This structure is not mandatory, but the idea is mandatory:
stop stacking everything inside a few giant files.
```
# 9. Migration Strategy Per Mechanic

For every mechanic, follow this exact process.

Step 1 — Find source truth
Identify:

original file(s)
related assets
related constants
related save data
related UI
related event hooks
Step 2 — Extract intent
Understand:

what problem the mechanic solves
what player experience it creates
what dependencies it needs
Step 3 — Decide implementation mode
One of:

direct port
clean rewrite
merged rewrite
simplified version
temporary stub with TODO
Step 4 — Integrate with target architecture
Do not dump raw old code if:

names are inconsistent
hidden dependencies exist
state ownership is unclear
update order assumptions are unsafe
Step 5 — Validate
Check:

compile
runtime behavior
no obvious regressions
no asset path breakage
no save corruption
no desync in state transitions

# 10. Mechanics Checklist Matrix

Use this table during migration.

10.1 Core
 Main loop stabilized
 Delta time / time scaling stable
 Input handling unified
 Asset path normalization
 Save/load boundary design
 Error logging pass
 
10.2 Player
 Movement
 Interaction
 Inventory
 Health
 Healing
 Reloading
 Stress
 SecondWind
 SoulLine
 Rations buffs
 
10.3 Enemy AI
 Base hostile AI
 VerminRush
 GhoulRush
 HumanTactical
 RobotControl
 Awareness buildup
 Aggro
 Attack cooldowns
 Localized mechanical damage
 
10.4 Vehicle / Titan
 Mount
 Dismount
 Cockpit fire
 Seat swap
 Thermal load
 Bucket Rig
 Ram Shield
 Tow Coupler
 Damage state integration
 Repair hooks
 
10.5 World
 Weather
 EtherFog
 AcidRain
 Breakables
 Shock wave effects
 Zone events
 Terminals
 Hangar
 Loot spawning
 
10.6 Progression / Meta
 Skills
 Story route
 Profile/session
 Save migration/versioning
 Pip-pad sync/customization
 Tape/radio system
 
10.7 Tools / Extra
 SpatialGrid
 ToolGun
 C.A.M.P.
 OBJ loading
 Prefab library
 World editor
 Networking review

# 11. Folder and Naming Normalization Rules

Current known issue:

assets/Enemies/ should become assets/enemies/
assets/Vehicles/ should become assets/vehicles/
General rule:

folder names use lowercase
avoid mixed-case asset paths
match include paths exactly
no duplicate assets only differing by case
all runtime references must be updated with rename
Why:

Linux/macOS case sensitivity will break builds silently otherwise

# 12. Immediate Technical Debt To Eliminate

12.1 Giant god-files
If logic is buried in huge files:

split by domain
keep interfaces small
move constants to dedicated config structures
12.2 Hidden state mutation
Avoid mechanics where multiple systems directly edit same fields without ownership rules.

12.3 Hardcoded asset paths everywhere
Centralize asset path constants.

12.4 Undocumented magic numbers
Convert to named constants/config values.

12.5 Broken feature stubs
If a feature is fake/broken:

either disable clearly
or implement properly
but do not leave deceptive half-working code

# 13. Testing Policy During Migration

We must test after each mechanic batch.

13.1 Compile test
clean compile
no newly introduced warnings if practical
no missing includes
no broken case-sensitive paths

13.2 Runtime smoke test
launch game
spawn player
move
interact
attack
load map
no immediate crash

13.3 Mechanic-specific tests
Examples:

AI sees player only after awareness threshold
reload blocks fire until complete
healing restores expected HP
AcidRain applies damage over time
seat swap preserves vehicle state
thermal load penalizes firing correctly

13.4 Regression checklist
After each batch:

player still moves
camera still works
collision still works
no null access on destroyed entities
no broken save/load if implemented

# 14. Suggested Execution Order for Actual Code Work

This is the recommended practical order.

Batch 1
normalize asset folder case
stabilize main.cpp
confirm TitanAI.hpp integration
reduce bootstrap complexity

Batch 2
create enemy AI/perception module
restore hostile archetypes
restore awareness/aggro
restore attack cooldowns/damage behavior

Batch 3
restore healing/reload/rations
restore stress/second wind/soulline

Batch 4
restore titan/vehicle advanced mechanics
mount/dismount
seat swap
cockpit fire
thermal load
utility modules

Batch 5
restore weather and breakables
add shockwave interactions
restore loot generator

Batch 6
restore terminals, hangar, zone events
restore tape/radio
restore story progression

Batch 7
spatial grid
camp/build system
toolgun
obj loading
editor/prefabs

Batch 8
evaluate networking separately
likely postpone until single-player gold state

# 15. What Counts as "Done"

The project is only considered done when all are true:

all important mechanics from source repos are accounted for
target repo builds cleanly
target repo runs without major blocking bugs
final gameplay loop is complete
titan/vehicle gameplay is functional
enemy gameplay is functional
survival systems are functional
world interactions are functional
no known catastrophic path/case issues remain
major content/story triggers work
remaining missing features are documented explicitly

# 16. Current Next Action

Immediate next implementation target after this plan:

verify and normalize repository structure
update/fix:
include/TitanAI.hpp
main.cpp
rename asset folders:
assets/Enemies/ -> assets/enemies/
assets/Vehicles/ -> assets/vehicles/
begin missing-mechanics recovery with:
HostileAI
Awareness/Aggro
Heal/Reload
Stress/SecondWind/SoulLine
vehicle interaction core

# 17. Working Notes

Important reminder:

source repos contain bugs
some mechanics should be rewritten, not blindly copied
feature completeness matters, but stability matters more
if two source systems conflict, preserve the better gameplay behavior and cleaner architecture
all migration decisions should be documented

# 18. Status Ledger Template

Copy this section and update during work.

Current Status
Audit: IN PROGRESS
Core stabilization: NOT STARTED
AI migration: NOT STARTED
Vehicle systems: PARTIAL
Player survival systems: NOT STARTED
World systems: NOT STARTED
Progression/story systems: NOT STARTED
Tool/editor systems: NOT STARTED
Networking review: NOT STARTED
Last completed task
Created master migration plan
Next concrete task
Implement/fix first core code batch in target repo

