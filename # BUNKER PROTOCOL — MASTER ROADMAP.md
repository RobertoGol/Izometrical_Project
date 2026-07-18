# BUNKER PROTOCOL — MASTER ROADMAP

## Текущее состояние

### Уже реализовано

* 3D-рендер сцены
* Камера (FPS/Orbit)
* Загрузка `.bwld` миров
* Отрисовка объектов мира
* Базовое перемещение игрока
* CMake + MSVC сборка
* GLFW + OpenGL + ImGui
* Основа `World/MapObject`
* Save/Load foundations
* Loot tables
* Inventory foundations
* Equipment foundations
* Weather/Event timers
* File Registry
* Export_data scanner
* UI логика инвентаря и контейнеров
* Основа языка `Lang_for_my_decisions`

# ACTIVE CHECKLIST

This is the single active checklist for unfinished implementation and verification work. Older roadmap sections below keep project context and completion history, but active open items should be added here.

## Completed Foundation Checkpoint

- [x] Door/Pickup/CraftingStation/NPC interaction execution.
- [x] Region-based external loot table loading.
- [x] Loot respawn rules with world/session timer policy.
- [x] DoorTransition loading foundation: fade phases, loader callback boundary, `.bwld` existence fallback, and destination world handoff.
- [x] Fullscreen DoorTransition fade overlay.
- [x] Equipment save/load verification against `ModularEquipmentSystem`.
- [x] Persist opened doors and current `.bwld` world id.
- [x] Add real `.bwld` parser/assets and wire the DoorTransition loader callback to world unload/load when map files exist.
- [x] Finish MapEditor ownership/size review after earlier `main.cpp` split.
- [x] Review remaining string parser chains for dispatch-table extraction where it improves readability.
- [x] Player slide, wall blocking, and 3D camera movement.
- [x] Mouse aim/toolgun/CAMP placement world position.
- [x] HUD/PipPad/Terminal visible over 3D.
- [x] Ground, terrain and cube material colors.
- [x] Save/load preserves new 3D/ECS data.

## P1 - Vertical Slice

- [ ] Build full Inventory UI: slots, stack display, item actions, drag/drop, stack split, transfer flow, and tooltip rendering.
- [ ] Build Container UI connected to loot containers, loose pickups, inventory transfer, weight limits, and save/load state.
- [ ] Build Equipment UI/panels for weapons, armor, ammo, BT-72 parts, durability, weight, and quick comparison.
- [ ] Finish Equipment System gameplay rules: equip/unequip, durability damage, repair hooks, ammo compatibility, armor effects, and weight penalties.
- [ ] Finish ECS foundations: add/verify render, physics, inventory, interaction, health, and script components where they are still represented only by legacy `GameState`.
- [ ] Add explicit GameState/GameLoop modes for Loading, Playing, Paused, Inventory, Terminal, Dialogue, and Menu so UI screens do not fight gameplay input.
- [ ] Integrate `Lang_for_my_decisions` VM core: parser/AST, bytecode interpreter, variables, script context, native bindings, and terminal command bridge.
- [ ] Connect terminal/workstation actions to real UI flows instead of placeholder/autocraft-only execution.

## P1.5 - First Playable Combat And Health

- [ ] Complete weapon combat: hitscan, projectiles, recoil, spread, reload timing, ammo consumption, critical hits, armor penetration, explosions, and limb damage.
- [ ] Finish enemy damage feedback: hit reactions, death states, loot drops, score/XP rewards, and readable combat logs/HUD events.
- [ ] Add player health consequences: bleeding, healing item flow, armor mitigation, death/fail state, and respawn/load recovery.
- [ ] Verify Titan/BT-72 cockpit combat: cannon/autocannon, missiles, heat, utility seat behavior, and cockpit HUD feedback.

## P2 - Core Game Systems

- [ ] Expand Crafting System: recipe database, ingredient validation, station UI, repair recipes, upgrade recipes, and crafting result feedback.
- [ ] Expand Survival System: hunger, thirst, fatigue, sleep, temperature, radiation, diseases, bleeding, and medical items.
- [ ] Expand Weather and World Events: scheduler, radiation/ether storms, visibility modifiers, temperature effects, random events, and world event persistence.
- [ ] Finish Pip-Pad as the main in-game tablet: status, inventory, equipment, map, radio, quests, tapes/logs, and readable Fallout-style layout.
- [ ] Finish Game HUD: health, energy, armor, radiation/erosion, weapon, ammo, active effects, crosshair, interaction prompts, subtitles, and state-specific HUD variants.
- [ ] Finish in-game pause/menu flow: resume, save, load, settings, quit to menu, and modal input blocking.
- [ ] Finish save/load UI: save slots, overwrite confirmation, load preview, autosave/manual save labels, and corrupted/missing save handling.
- [ ] Finish loading screens for new game, continue, `.bwld` transitions, and save/load restore.

## P3 - Unique Bunker Protocol Systems

- [ ] Build BT-72/AIMP deep systems: energy blocks, power modes, internal storage, terrain scanner, radar, heat/overheat, module damage, upgrade tree, and autopilot.
- [ ] Build AIMP personal base system: storage, production, repair, upgrades, equipment setup, medical/research/defense/energy modules, and expedition management.
- [ ] Build Quest System: quest database, objectives, triggers, rewards, branching choices, fail conditions, journal UI, and save/load persistence.
- [ ] Build Fog of Information: unexplored map, discovery by towers/scanners, overwatch points, radar reveal zones, and Pip-Pad map integration.
- [ ] Build Expedition Planning UI: team/loadout selection, risk/reward preview, region choice, supply costs, launch flow, and return results.

## P4 - NPC, AI, Factions, Dialogue

- [ ] Expand AI System: navmesh/pathing, patrol, alert/search/combat/flee states, cover system, squad behavior, perception tuning, and debug visualization.
- [ ] Build Factions and Economy: reputation, trading, dynamic prices, resource scarcity, repair services, contracts, expedition rewards, and faction save data.
- [ ] Build Dialogue System: dialogue trees, conditions, reputation checks, quest hooks, voice/log integration, companion dialogue, and terminal/Pip-Pad history.
- [ ] Expand neutral NPCs beyond placeholders: schedules/locations, greetings, quest roles, vendor roles, and persistence.

## P5 - Co-op Model

- [ ] Implement host session flow with host-owned world state.
- [ ] Implement client join/rejoin for 2-4 players over the selected LAN/P2P/Steam path.
- [ ] Sync player transforms, inventory, containers, doors, enemies, quests, AIMP state, chat, and pings.
- [ ] Define co-op save rules: host world save, per-player character/AIMP progress, disconnect handling, and version compatibility.
- [ ] Build multiplayer UI: host, join, session browser/direct join, player list, invite/status, and connection failure states.

## P6 - Optimization And AA Polish

- [ ] Add chunk streaming for worlds and large maps.
- [ ] Add frustum culling, occlusion culling, LOD, and instanced rendering.
- [ ] Add async asset loading, texture streaming, mesh streaming, shader cache, and resource lifetime tracking.
- [ ] Add profiler, performance overlay, frame-time budget checks, memory budget checks, and crash reporter.
- [ ] Add graphics settings: resolution, fullscreen/windowed, VSync, quality presets, AA, shadows, effects, FOV, brightness/gamma.
- [ ] Add audio settings: master/music/SFX/voice volume, language, subtitles, subtitle size, and speaker mode.
- [ ] Add controls settings: key rebinding, mouse sensitivity, controller support, invert axis, and accessibility toggles.

## Player Flow And Presentation

- [ ] Build Main Menu: Continue, New Game, Load Game, Settings, Credits, Exit.
- [ ] Build Continue preview: character name, level, play time, last region, AIMP state, and BT-72 state.
- [ ] Build New Game flow: character creation, background selection, personal AIMP creation, and first world loading.
- [ ] Build Load Game screen: save slots, metadata previews, delete/rename where useful, and safe fallback for missing/corrupt saves.
- [ ] Build Character Creation: appearance, face, hair, body, identity/name, background, attributes, and confirmation flow.
- [ ] Build Character Profile persistence for name, appearance choices, background, level, skills, equipment, and play time.
- [ ] Build Character Menu and status screens: stats, health, conditions, active effects, equipment, and readable progression summary.
- [ ] Build Skills/Perks UI and unlock flow for Engineering, Combat, Survival, Science, and Leadership.
- [ ] Implement progression rules: XP gain, level-up, skill points, unlock requirements, specialization/respec policy, and save/load.
- [ ] Build BT-72/AIMP control interface: modules, energy, armor, scanner, upgrades, storage, repair, and autopilot status.

## Content, Tools, And Data Pipeline

- [ ] Finish MapEditor export/import path for runtime `.bwld` format and verify round-trip with `WorldFileLoader`.
- [ ] Expand `.bwld` schema for objects, materials, spawn sets, triggers, region metadata, nav data, lighting, and scripted events.
- [ ] Add asset validation for materials, textures, audio references, loot tables, enemies, vehicles, workstations, and `.bwld` files.
- [ ] Finish code transfer/integration still useful from `Video_Game_Izom`: inventory UI, container UI, equipment slots, tooltip system, and quest journal UI.
- [ ] Finish code transfer/integration still useful from `Lang_for_my_decisions`: parser, AST, VM core, native binding layer, and terminal command system.
- [ ] Review what remains useful from `GMyGameDoNotTouch`: weather scheduler, world metadata, event timers, file registry, and any non-duplicated save helpers.

## Final Verification Before Release Builds

- [ ] Add smoke tests or test harnesses for save/load, `.bwld` load, inventory transfer, crafting, equipment, combat, UI modes, and material catalog validation.
- [ ] Add manual QA checklist for movement/collision, doors, loot, pickups, NPCs, combat, crafting, CAMP, Pip-Pad, terminal, map, save/load, and settings.
- [ ] Verify keyboard/mouse input conflicts across gameplay, inventory, terminal, map, pause menu, and dialogue.
- [ ] Verify UI readability at target resolutions and no overlapping HUD/Pip-Pad/terminal/menu text.
- [ ] Verify branch builds cleanly on the expected Windows/MSVC setup and document any required installed tools.

# P0 — КРИТИЧЕСКИЙ МИНИМУМ

## 1. CollisionSystem

**Приоритет:** P0

### Нужно

* Capsule/AABB игрока
* Sweep collision
* Slide along walls
* Ground detection
* Ceiling detection

### Файлы

* `CollisionSystem.hpp`
* `CollisionSystem.cpp`
* `PhysicsWorld.hpp`
* `PhysicsWorld.cpp`

### Status

* [x] Added `PhysicsWorld` tile-solid query layer over current `GameState`.
* [x] Added `CollisionSystem` sweep/slide movement result API.
* [x] Added ground and ceiling contact flags for the 3D movement pipeline.
* [x] Routed player walk/dive movement through `CollisionSystem::sweepAndSlide`.
* [x] Routed dynamic collision resolve through `CollisionSystem::resolveDynamicCollisions`.
* Active QA item moved to `ACTIVE CHECKLIST`.

---

## 2. InteractionManager

**Приоритет:** P0

### Нужно

* Raycast из камеры
* Выбор объекта
* Подсветка
* Клавиша `E`
* Distance check
* Line-of-sight check

### Типы взаимодействий

* Container
* Door
* Terminal
* Pickup
* CraftingStation
* NPC

### Status

* [x] Added `InteractionManager.hpp/.cpp`.
* [x] Added best-target query with distance checks.
* [x] Added line-of-sight checks through `PhysicsWorld`.
* [x] Routed `E` interaction through `InteractionManager`.
* [x] Connected current working interaction types: `Terminal`, `Container`, `Vehicle`.
* [x] Highlight rendering hook is connected to a visible `[E]` world overlay.
* Remaining interaction execution work moved to `ACTIVE CHECKLIST`.

### Файлы

* `InteractionManager.hpp`
* `InteractionManager.cpp`

---

## 3. LootManager

**Приоритет:** P0

### Нужно

* Weighted loot generation
* Container state persistence
* One-time loot generation
* Respawn rules
* Loot rarity
* Region-based loot tables

### Перенести из GMyGameDoNotTouch

* `LootEntry`
* `LootTable`
* Roll logic
* Serialization helpers

### Status

* [x] Added `LootManager.hpp/.cpp`.
* [x] Centralized weighted tier rolls and container filling.
* [x] Preserved old `LootGenerator` name as a compatibility wrapper.
* [x] Added container-tier mapping for wooden crates, iron safes and dev vaults.
* [x] One-time generation respects existing container contents and opened state.
* [x] Container state persistence remains covered by existing `SaveSystem` container serialization.
* Remaining loot loading and respawn policy work moved to `ACTIVE CHECKLIST`.

### Файлы

* `LootManager.hpp`
* `LootManager.cpp`
* `LootTables.hpp`

---

## 4. DoorTransition

**Приоритет:** P0

### Нужно

* `linkTarget`
* Destination spawn point
* Async world loading
* Fade in/out
* World unload

### Status

* [x] Added `DoorTransition.hpp/.cpp`.
* [x] Added door links with `linkTarget` and destination spawn point.
* [x] Added transition phases: fade out, loading, fade in.
* [x] Connected door targets to `InteractionManager` and `E` interaction.
* [x] Connected transition update to the game loop.
* Remaining world-loader and fade overlay work moved to `ACTIVE CHECKLIST`.

### Файлы

* `DoorTransition.hpp`
* `DoorTransition.cpp`

---

## 5. SaveGame

**Приоритет:** P0

### Нужно сохранять

* Позицию игрока
* Инвентарь
* Экипировку
* Состояние контейнеров
* Открытые двери
* Текущее время мира
* Текущий `.bwld` мир

### Перенести из GMyGameDoNotTouch

* `SessionProfile`
* Save versioning
* Metadata serialization
* Binary/JSON helpers

### Status

* [x] Added `SaveGame.hpp/.cpp` facade over the existing binary `SaveSystem`.
* [x] Routed app autosave, restore, manual save and manual load through `SaveGame`.
* [x] Player position, inventory, containers, world grid, progression/story and ECS runtime data remain covered by current serialization.
* [x] Save versioning remains handled by `SaveSystem`.
* Remaining equipment and world-door persistence work moved to `ACTIVE CHECKLIST`.

### Файлы

* `SaveGame.hpp`
* `SaveGame.cpp`

---

# P1 — ВЕРТИКАЛЬНЫЙ СРЕЗ

## 6. Inventory UI

**Приоритет:** P1

### Перенести из Video_Game_Izom

* Slot logic
* Tooltip rendering
* Equipment panels
* Drag & Drop
* Stack split

---

## 7. Equipment System

**Приоритет:** P1

### Нужно

* Оружие
* Броня
* Боеприпасы
* Weight system
* Durability

---

## 8. ECS Foundations

**Приоритет:** P1

### Компоненты

* `TransformComponent`
* `RenderComponent`
* `PhysicsComponent`
* `InventoryComponent`
* `InteractionComponent`
* `HealthComponent`
* `ScriptComponent`

---

## 9. GameState / GameLoop

**Приоритет:** P1

### Состояния

* Loading
* Playing
* Paused
* Inventory
* Terminal
* Dialogue

---

## 10. Lang_for_my_decisions VM

**Приоритет:** P1

### Нужно

* VM core
* Bytecode interpreter
* Variable storage
* Native function binding
* Script context

### Game API

* `openDoor(id)`
* `closeDoor(id)`
* `spawnItem(id)`
* `playSound(id)`
* `startQuest(id)`
* `completeQuest(id)`
* `setFlag(name, value)`
* `getFlag(name)`

### Компоненты

* `TerminalScriptComponent`
* `DoorScriptComponent`
* `QuestScriptComponent`
* `TriggerScriptComponent`

---

# P2 — ОСНОВНАЯ ИГРА

## 11. Combat System

**Приоритет:** P2

### Нужно

* Hitscan weapons
* Projectile weapons
* Damage calculation
* Armor penetration
* Critical hits
* Limb damage
* Recoil
* Reloading
* Weapon spread
* Explosion damage

---

## 12. Crafting System

**Приоритет:** P2

### Нужно

* Recipe database
* Ingredient validation
* Crafting stations
* Repair recipes
* Upgrade recipes

---

## 13. Survival System

**Приоритет:** P2

### Нужно

* Hunger
* Thirst
* Fatigue
* Sleep
* Temperature
* Radiation
* Diseases
* Bleeding
* Medical items

---

## 14. Weather & World Events

**Приоритет:** P2

### Перенести из GMyGameDoNotTouch

* Weather scheduler
* Radiation storms
* Visibility modifiers
* Temperature effects
* Random events

---

# P3 — УНИКАЛЬНЫЕ МЕХАНИКИ BUNKER PROTOCOL

## 15. BT-72 System

**Приоритет:** P3

### Нужно

* Energy blocks
* Power modes
* Terrain scanner
* Radar
* Internal storage
* Heat / Overheat
* Module damage
* Upgrade tree
* Autopilot

### Системы

* `BT72CoreComponent`
* `BT72EnergySystem`
* `BT72Inventory`
* `BT72UpgradeTree`

---

## 16. Quest System

**Приоритет:** P3

### Нужно

* Quest database
* Objectives
* Triggers
* Rewards
* Branching choices
* Fail conditions
* Journal UI

---

## 17. Fog of Information

**Приоритет:** P3

### Нужно

* Неизведанная карта
* Открытие территории через башни/сканеры
* Overwatch points
* Radar reveal zones

---

# P4 — NPC И ФРАКЦИИ

## 18. AI System

**Приоритет:** P4

### Нужно

* NavMesh
* Patrol
* Alert state
* Search state
* Combat state
* Flee state
* Cover system
* Squad behavior

### Файлы

* `AISystem.hpp`
* `BehaviorTree.hpp`
* `NavMesh.hpp`

---

## 19. Factions & Economy

**Приоритет:** P4

### Нужно

* Reputation
* Trading
* Dynamic prices
* Resource scarcity
* Repair services
* Contracts
* Expedition rewards

---

## 20. Dialogue System

**Приоритет:** P4

### Нужно

* Dialogue trees
* Conditions
* Reputation checks
* Voice/log integration
* Companion dialogue

---

# P5 — КООПЕРАТИВ (модель State of Decay 2)

## Архитектура

* Один игрок = Host
* Мир хранится у Host
* 2–4 клиента
* P2P / LAN / Steam Join

## Нужно

* Session Host
* Client Join
* Player Transform Sync
* Inventory Sync
* Container Sync
* Door State Sync
* Enemy State Sync
* Quest Progress Sync
* Chat / Ping System
* Rejoin Session

## Важно

* Dedicated servers НЕ нужны
* MMO-инфраструктура НЕ нужна
* Server authority НЕ нужна

---

# P6 — ОПТИМИЗАЦИЯ И AA-УРОВЕНЬ

## Нужно

* Chunk Streaming
* Frustum Culling
* Occlusion Culling
* LOD System
* Async Asset Loading
* Instanced Rendering
* ResourceManager
* Texture Streaming
* Mesh Streaming
* Shader Cache
* Profiler
* Crash Reporter

---

# ПЕРЕНОС КОДА

## Из GMyGameDoNotTouch

1. `SessionProfile`
2. Loot tables
3. Save/Load helpers
4. Weather scheduler
5. World metadata
6. Event timers
7. File Registry

## Из Video_Game_Izom

1. Inventory UI
2. Container UI
3. Equipment slots
4. Tooltip system
5. Quest journal UI

## Из Lang_for_my_decisions

1. Parser
2. AST
3. VM core
4. Native binding layer
5. Terminal command system

---

# ПОРЯДОК РАЗРАБОТКИ

1. P0 — полностью
2. P1 — полностью
3. P2 — Combat + Survival + Weather
4. P3 — BT-72 + Quests
5. P4 — NPC + AI
6. P5 — Кооператив
7. P6 — Оптимизация

---

# РЕЗУЛЬТАТЫ ПО ЭТАПАМ

## После P0

Игрок может ходить, сталкиваться со стенами, открывать двери, лутать контейнеры, сохраняться и загружаться.

## После P1

Полноценный вертикальный срез с инвентарём, ECS и терминалами на `Lang_for_my_decisions`.

## После P2

Игра ощущается как Fallout-подобный survival RPG.

## После P3

Появляется уникальная идентичность Bunker Protocol (BT-72 + Fog of Information).

## После P4

Мир становится живым за счёт NPC, фракций и экономики.

## После P5

Рабочий кооператив в модели State of Decay 2.

## После P6

Большой открытый мир с производительностью уровня AA-проекта.

---

# ТЕКУЩАЯ ОЦЕНКА ГОТОВНОСТИ

| Система       | Готовность |
| ------------- | ---------- |
| 3D Engine     | 85%        |
| World Loading | 90%        |
| Rendering     | 85%        |
| Gameplay Core | 35–40%     |
| Persistence   | 60%        |
| UI            | 50%        |
| AI            | 10%        |
| Combat        | 15%        |
| BT-72         | 15%        |
| Scripting VM  | 25%        |
| Co-op         | 5%         |

## Общая готовность

* До MVP: ~55%
* До V1: ~35–40%
* До полной версии: ~20–25%

# ДОПОЛНЕНИЕ ПОСЛЕ ПРОВЕРКИ РЕКОМЕНДАЦИЙ

## Изменение приоритетов

### Базовый Combat переносится выше

**Было:** P2
**Стало:** P1.5 (между P1 и P2)

### Причина
Без стрельбы вертикальный срез ощущается как редактор мира с инвентарём, а не как Fallout-подобная игра.

### Минимальный набор Combat для первого играбельного билда
- Hitscan оружие
- Урон по цели
- Перезарядка
- Боеприпасы
- HP игрока
- Смерть игрока
- Лечение аптечкой

### Файлы
- CombatSystem.hpp
- CombatSystem.cpp
- HealthComponent.hpp
- WeaponComponent.hpp

---

## Что можно отложить до после первого играбельного билда

### Survival Advanced
- Diseases
- Temperature
- Bleeding
- Infections
- Advanced medical effects

### Economy Advanced
- Dynamic prices
- Resource scarcity simulation
- Repair services
- Contracts

### AI Advanced
- Companions
- Squad behavior
- Procedural encounters
- Daily schedules

---

## Что убрать из ближайшего плана (до v1)

### Engine Optimization
- Occlusion Culling
- World Origin Shifting
- Texture Streaming
- Mesh Streaming
- Shader Cache
- Job System
- Multithreaded Loading
- Instanced Rendering
- Crash Reporter
- Profiler

Эти задачи важны, но не приближают проект к первому играбельному билду.

---

# ОБНОВЛЁННЫЙ ПОРЯДОК РАЗРАБОТКИ

1. P0 — полностью
2. P1 — полностью
3. P1.5 — Combat + Health
4. P2 — Survival + Crafting + Weather
5. P3 — BT-72 + Quests
6. P4 — NPC + AI
7. P5 — Co-op (State of Decay 2 модель)
8. P6 — Optimization

---

# РЕАЛИСТИЧНЫЙ ПЛАН НА БЛИЖАЙШИЕ 6 НЕДЕЛЬ

## Неделя 1
- CollisionSystem

## Неделя 2
- InteractionManager

## Неделя 3
- LootManager + Container UI

## Неделя 4
- DoorTransition + SaveGame

## Неделя 5
- Inventory + Equipment

## Неделя 6
- Combat + Health + Lang_for_my_decisions VM

---

# РЕЗУЛЬТАТ ПОСЛЕ ЭТОГО ЭТАПА

Игрок сможет:
- Исследовать мир
- Открывать двери
- Лутать контейнеры
- Стрелять
- Получать урон
- Умирать и лечиться
- Сохранять и загружать игру
- Использовать терминалы через Lang_for_my_decisions

Это и будет первый настоящий играбельный билд Bunker Protocol.


---

# PLAYER EXPERIENCE, UI AND GAME FLOW ROADMAP

## Общая концепция

Bunker Protocol создаётся как survival RPG с опытом, вдохновлённым:

- Fallout 4
- Fallout 76
- State of Decay 2
- Titanfall 2
- другими играми с сильным ощущением выживания, экспедиции и управления ресурсами.

Главная цель:

Игрок должен ощущать не просто запуск технического прототипа, а полноценное путешествие:


Game Launch
↓
Main Menu
↓
Character Creation
↓
AIMP Selection / Creation
↓
World Loading
↓
Gameplay
↓
Exploration
↓
Combat
↓
Loot
↓
Progression
↓
Expansion of AIMP


---

# CURRENT MISSING PLAYER SYSTEMS

## Системы, которых сейчас не хватает

### Main Menu

Отсутствует полноценный входной экран игры.

Необходимо создать:

- Continue
- New Game
- Load Game
- Settings
- Credits
- Exit

---

# MAIN MENU SYSTEM

## Continue

Загрузка последнего состояния.

Отображать:

- имя персонажа;
- уровень;
- время игры;
- последний регион;
- состояние AIMP;
- состояние BT-72.

Пример:


CONTINUE

Robert

Level 12

AIMP: Northern Camp

Play Time:
38h 24m


---

# New Game

Новая игра.

Порядок:


New Game

↓

Character Creation

↓

Character Background

↓

Create Personal AIMP

↓

Starting Location

↓

World Introduction


---

# Load Game

Экран сохранений.

Каждое сохранение должно показывать:

- персонажа;
- уровень;
- дату;
- место;
- состояние мира;
- состояние AIMP;
- последний прогресс.

---

# SETTINGS MENU

## Graphics

Настройки:

- Resolution
- Fullscreen / Window Mode
- VSync
- FPS Limit
- Texture Quality
- Shadow Quality
- View Distance
- Effects Quality

---

## Audio

Настройки:

- Master Volume
- Music
- Effects
- Voice
- Radio
- Ambient Sounds

---

## Controls

Настройки управления:

Пример:


Move WASD

Interact E

Pip-Pad TAB / P

Inventory I

Reload R

Jump SPACE

Map M


---

## Gameplay

Настройки:

- Difficulty
- HUD visibility
- Auto-save
- Tutorials
- Interaction hints
- Damage indicators

---

## Accessibility

Настройки:

- Subtitle size
- Text scaling
- Color options
- Interface scaling

---

# CHARACTER CREATION SYSTEM

## Общая идея

Каждый игрок создаёт своего собственного персонажа.

В кооперативном режиме:

- Host не создаёт персонажей за других игроков.
- Каждый игрок имеет своего персонажа.
- Каждый игрок имеет свой прогресс персонажа.
- Каждый игрок имеет свой набор навыков и экипировки.

---

# Character Creation Flow


Create Character

↓

Appearance

↓

Attributes

↓

Background

↓

Name

↓

Create AIMP

↓

Start Game


---

# Appearance Editor

Вдохновение:

Fallout 4 character creator.

Настройки:

## Face

- Head shape
- Jaw
- Nose
- Eyes
- Mouth
- Skin details

---

## Hair

- Hair style
- Color
- Facial hair

---

## Body

- Height
- Body type
- Build

---

# Character Identity

## Name

Игрок задаёт имя.

Пример:


Name:
Robert


---

# Background System

Перед стартом игрок выбирает прошлое.

Примеры:

## Engineer

Бонусы:

+ ремонт техники
+ создание модулей

Минусы:

- меньше боевых бонусов


---

## Explorer

Бонусы:

+ поиск ресурсов
+ исследование карты


---

## Soldier

Бонусы:

+ оружие
+ тактика

---

# Attribute System

Вариант Fallout SPECIAL.

Для Bunker Protocol возможно использовать:


Strength

Engineering

Survival

Science

Tactics

Mobility

Leadership


---

# AIMP SYSTEM

## Personal Camp System

В отличие от Fallout:

Игрок не имеет один общий дом.

Вдохновение:

State of Decay 2.

---

## Каждый игрок имеет собственный AIMP

AIMP:

Autonomous Integrated Mobile Platform / Personal Camp System

Это личная база игрока.

---

## В кооперативе:


Player 1
|
AIMP 1

Player 2
|
AIMP 2

Player 3
|
AIMP 3


---

# AIMP Features

## Base Management

Возможности:

- хранение ресурсов;
- производство;
- ремонт;
- улучшение;
- настройка оборудования;
- управление экспедициями.

---

## AIMP Modules

Примеры:

- Storage Module
- Workshop Module
- Medical Module
- Research Module
- Defense Module
- Energy Module

---

# GAME HUD SYSTEM

## Игровой интерфейс

Во время игры нужен постоянный HUD.

Отображать:

- здоровье;
- энергию;
- состояние брони;
- радиацию;
- оружие;
- боезапас;
- активные эффекты.

---

Пример:


HP

ENERGY

RADIATION

    CROSSHAIR

WEAPON

AMMO


---

# PIP-PAD SYSTEM

## Игровой планшет

Pip-Pad является главным устройством игрока.

Не просто меню.

Это часть мира.

---

# Pip-Pad Sections

## Status

Информация:

- здоровье;
- состояние тела;
- эффекты;
- травмы.

---

## Inventory

Раздел:

- предметы;
- ресурсы;
- оружие;
- материалы.

---

## Equipment

Раздел:

- оружие;
- броня;
- модули.

---

## Map

Особенность:

Карта изначально неизвестна.


UNKNOWN AREA


После исследования:


DISCOVERED REGION


---

## Radio

Система:

- сигналы;
- сообщения;
- события;
- предупреждения.

---

## Quests

Журнал:

- Main Quest
- Side Quest
- Tasks

---

# IN-GAME MENU

Отдельно от Main Menu.

Открывается во время игры.

Пример:


Resume

Save

Load

Settings

Exit


---

# SAVE SYSTEM UI

Экран сохранений должен учитывать:

- персонажа;
- AIMP;
- мир;
- время;
- найденные места;
- выполненные задачи.

---

# LOADING SCREEN SYSTEM

Загрузка должна использоваться как часть атмосферы.

Показывать:

- советы;
- информацию о мире;
- состояние оборудования;
- историю.

Пример:


BT-72 POWER CORE

Energy:
82%

Current Mission:
Northern Expedition


---

# CHARACTER PROGRESSION

## Level System

После получения опыта:


LEVEL UP

Choose Upgrade


---

# Skills / Perks

Примеры:

## Engineering Rank 1

Позволяет:

- ремонтировать оборудование.

## Engineering Rank 2

Позволяет:

- создавать улучшенные модули.

---

# BT-72 / AIMP CONTROL INTERFACE

Отдельное меню управления.

Пример:


BT-72 STATUS

Power:
82%

Heat:
34%

Damage:
12%

Modules:

Scanner
Storage
Armor
Engine


---

# EXPEDITION PLANNING SYSTEM

Перед выходом игрок готовит экспедицию.

Пример:


Destination:

Northern Factory

Required:

Food
Ammo
Repair Kit
Energy


---

# CO-OP MODEL

## Архитектура

Модель:

State of Decay 2.

Не MMO.

---

## Основные правила:

- Dedicated servers не нужны.
- Каждый игрок создаёт своего персонажа.
- Каждый игрок имеет собственный AIMP.
- Host управляет состоянием мира.
- Клиенты подключаются к существующему миру.

---

# CO-OP PLAYER STRUCTURE

Пример:


HOST

World State

AI

Events

Loot

PLAYER 1

Character

Inventory

AIMP

PLAYER 2

Character

Inventory

AIMP


---

# PRIORITY ORDER FOR PLAYER SYSTEMS

## P0

- Main Menu
- Pause Menu
- Settings
- Save/Load UI


## P1

- Character Creation
- Basic HUD
- Inventory UI
- Pip-Pad


## P2

- Perks
- Skills
- Dialogue UI
- Quest Journal


## P3

- AIMP Management
- BT-72 Interface
- Expedition Planning


---

# FINAL GOAL

Игрок должен чувствовать:


I create my survivor.

I build my AIMP.

I explore the world.

I collect resources.

I upgrade my equipment.

I survive dangerous expeditions.

I build my own story.


Bunker Protocol должен быть не просто техническим движком.

Он должен стать полноценной survival RPG системой, где интерфейс, персонаж, база и мир работают как единая экосистема.
---

# CHARACTER SKILL SYSTEM REVIEW AND IMPROVEMENT PLAN

## Общий статус системы персонажа

На текущем этапе разработки система персонажа уже имеет базовый фундамент, но ещё не полностью превращена в игровую механику.

Сейчас реализовано:

- два отдельных класса развития:
  - основной персонаж игрока;
  - Titan / Tank (тяжёлая боевая платформа).

- существует система:
  - XP (опыт);
  - Level (уровень);
  - базовая логика развития;
  - хранение навыков на уровне кода.

Пока отсутствует:

- UI отображение навыков;
- дерево развития;
- выбор навыков игроком;
- полноценная система Perks;
- связь навыков с игровыми действиями;
- связь навыков с AIMP и Titan.

Текущая система является фундаментом, который необходимо расширить до полноценной RPG-системы.

---

# CURRENT CHARACTER ARCHITECTURE

## Player Character

Основной персонаж игрока отвечает за:

- исследование мира;
- взаимодействие с объектами;
- бой;
- выживание;
- использование технологий;
- развитие навыков.

Развитие персонажа должно влиять на:

- доступные действия;
- эффективность действий;
- открытие новых возможностей;
- взаимодействие с миром.

---

## Titan / Tank System

Titan является отдельной системой развития.

Titan не должен использовать полностью такую же систему навыков, как обычный персонаж.

Titan представляет собой:

- тяжёлую боевую платформу;
- мобильную базу;
- транспорт;
- инструмент исследования;
- часть игрового прогресса.

Развитие Titan должно происходить через:

- модули;
- улучшения;
- энергию;
- оборудование;
- технические навыки персонажа.

---

# CURRENT PROGRESSION SYSTEM

## Как работает сейчас

Текущая логика:


Player Actions

↓

XP

↓

Level Up

↓

Получение развития


Система уже позволяет создавать основу прогресса персонажа.

Проблема:

Сейчас развитие больше техническое, чем игровое.

Игрок пока не ощущает:

- специализацию;
- выбор направления;
- уникальный стиль игры.

---

# PLANNED PROGRESSION SYSTEM

## Новая модель развития

Предлагается использовать смешанную систему.

Основы:

Fallout 4:

- SPECIAL;
- Perks;
- развитие через уровни.

Fallout 76:

- специализация;
- разные сборки персонажей;
- разные игровые стили.

Bunker Protocol:

- собственные направления развития;
- связь с Titan;
- связь с AIMP;
- влияние на мир.

---

# NEW CHARACTER DEVELOPMENT FLOW


Player Actions

↓

Experience (XP)

↓

Character Level

↓

Skill Points

↓

Skills / Perks

↓

New Abilities

↓

New Gameplay Options


---

# PLAYER SKILL CATEGORIES

## Engineering

Направление:

- технологии;
- ремонт;
- создание оборудования;
- работа с Titan;
- улучшение AIMP.

Пример развития:

### Engineering Level 1

Возможности:

- базовый ремонт оборудования.

### Engineering Level 2

Возможности:

- улучшение устройств;
- создание улучшенных деталей.

### Engineering Level 3

Возможности:

- модернизация Titan;
- создание сложных модулей.

---

# Combat

Направление:

- оружие;
- тактика;
- защита;
- боевые способности.

Примеры:

- снижение отдачи;
- повышение точности;
- новые боевые приёмы;
- эффективное использование оружия.

---

# Survival

Направление:

- выживание;
- исследование;
- ресурсы;
- опасные зоны.

Примеры:

- поиск дополнительных ресурсов;
- снижение расхода припасов;
- устойчивость к условиям мира.

---

# Science

Направление:

- исследования;
- технологии;
- терминалы;
- анализ предметов.

Примеры:

- доступ к закрытым системам;
- создание новых технологий;
- исследование неизвестных объектов.

---

# Leadership

Направление:

- управление AIMP;
- NPC;
- экспедиции;
- организация ресурсов.

Примеры:

- улучшение базы;
- управление персоналом;
- новые возможности лагеря.

---

# TITAN DEVELOPMENT SYSTEM

Titan получает отдельную систему развития.

Общая структура:


Titan

├── Core System
├── Armor
├── Energy
├── Weapons
├── Mobility
├── Scanner
├── Storage
└── Special Modules


---

# Titan Upgrade Examples

## Energy Module

Level 1:

- увеличение времени работы.

Level 2:

- повышение эффективности энергии.

Level 3:

- новые режимы питания.

---

## Armor Module

Level 1:

- повышение защиты.

Level 2:

- улучшенное восстановление.

Level 3:

- специальные защитные технологии.

---

## Scanner Module

Level 1:

- обнаружение ресурсов.

Level 2:

- анализ объектов.

Level 3:

- обнаружение скрытых технологий.

---

# CONNECTION BETWEEN PLAYER AND TITAN

Навыки персонажа должны влиять на Titan.

Пример:


Engineering Level 3

↓

Titan Advanced Repair

↓

New Upgrade Options


Игрок без нужного навыка не сможет использовать максимальный потенциал Titan.

---

# SKILL UI SYSTEM

Сейчас навыки существуют только в коде.

Следующий этап:

создать интерфейс развития персонажа.

---

# Character Menu

Структура:


Character

├── Status
├── Stats
├── Skills
├── Perks
├── Equipment
└── Background


---

# Skill Tree UI

Пример:


ENGINEERING

[1] Basic Repair

    ↓

[2] Advanced Repair

    ↓

[3] Titan Modification


---

# SKILL UNLOCK SYSTEM

Необходимо определить способ получения навыков.

---

## Option A

Через уровень персонажа.

Пример:


Level Up

↓

Choose Skill


Плюсы:

- понятно игроку;
- легко балансировать.

Минусы:

- меньше связи с действиями.

---

## Option B

Через использование навыков.

Пример:


Repair Objects

↓

Engineering XP

↓

Engineering Level Up


Плюсы:

- больше RPG ощущения.

Минусы:

- сложнее балансировать.

---

## Option C (Recommended)

Комбинация:


Character XP

Skill Usage

Training

↓

Skill Development


---

# SKILL LEARNING SYSTEM

Навыки могут открываться через:

- NPC обучение;
- терминалы;
- найденные записи;
- исследовательские станции;
- специальные события;
- редкие предметы.

---

# SAVE SYSTEM REQUIREMENTS

Система сохранения должна учитывать:

## Character:

- Level;
- XP;
- Skills;
- Perks;
- unlocked abilities.

---

## Titan:

- Level;
- Modules;
- Upgrades;
- Damage state;
- Equipment.

---

# LANG_FOR_MY_DECISIONS INTEGRATION

Навыки должны быть доступны игровым скриптам.

Пример:


if player.Engineering >= 3

unlock("AdvancedWorkshop")


Использование:

- квесты;
- терминалы;
- двери;
- события;
- технологии;
- проверки доступа.

---

# QUESTIONS BEFORE IMPLEMENTATION

## 1. Главный способ развития?

Выбрать:

A)
Только уровень персонажа.

B)
Только развитие через действия.

C)
Смешанная система.

---

## 2. Как развивается Titan?

Варианты:

A)
Titan получает собственный опыт.

B)
Titan развивается через навыки игрока.

C)
Комбинация двух вариантов.

---

## 3. Можно ли менять специализацию?

Например:

Engineer → Combat

Варианты:

- нельзя менять;
- можно через редкие предметы;
- можно всегда.

---

## 4. Максимальный уровень развития?

Нужно определить:

- количество уровней;
- количество навыков;
- глубину специализации.

---

## 5. Нужны ли ограничения?

Пример:

Engineer:

+

технологии

-

меньше боевых преимуществ.

---

# FINAL GOAL

После переработки система должна выглядеть так:


Player Actions

↓

XP

↓

Character Level

↓

Skills + Perks

↓

Character Build

Titan Development

AIMP Development

↓

Уникальный стиль игры каждого игрока


Главная цель:

Не копировать Fallout.

Использовать лучшие идеи Fallout 4/76 и создать собственную систему Bunker Protocol:


Player

↓

Skills

↓

Titan

↓

AIMP

↓

World Interaction


---

---
# BUNKER PROTOCOL
# UI / UX VISION DOCUMENT

## Для проектной команды, клиентов, менеджеров, игроков и коллег

---

# 1. Назначение документа

Этот документ описывает концепцию UI/UX системы игры **Bunker Protocol**.

Цель документа:

- объяснить направление развития интерфейса;
- описать внешний вид всех игровых меню;
- определить взаимодействие игрока с системой;
- показать, как UI связан с игровым миром;
- сформировать единый стиль для команды разработки, художников, программистов и менеджеров.

Основные источники вдохновения:

- Fallout 4;
- Fallout 76;
- Titanfall 2;
- State of Decay 2;
- survival RPG игры;
- military sci-fi интерфейсы.

Важно:

Bunker Protocol не копирует эти игры.

Мы используем лучшие идеи и создаём собственную систему:


Player

↓

Skills

↓

Equipment

↓

Titan

↓

AIMP

↓

World

↓

Player Story


---

# 2. Основная идея UI Bunker Protocol

Интерфейс должен быть не просто набором окон.

UI должен ощущаться как часть мира.

Игрок должен чувствовать:

"Я управляю своим оборудованием, базой и экспедицией."

Основные источники информации:

- Pip-Pad;
- HUD;
- Titan Interface;
- AIMP Interface;
- Terminal Systems;
- Equipment Systems.

---

# 3. Главные принципы UI/UX

## 3.1 Информация должна помогать принимать решения

Игрок получает:


Information

↓

Decision

↓

Action

↓

World Reaction


Пример:

Игрок видит:

- мало энергии Titan;
- плохое состояние оружия;
- недостаток ресурсов.

Игрок принимает решение:

- вернуться;
- искать ресурсы;
- изменить маршрут;
- подготовиться лучше.

---

## 3.2 UI должен быть частью атмосферы

Вместо обычного меню:

"Inventory"

используется:

"Personal Equipment Management System"

Вместо:

"Map"

используется:

"Navigation and Exploration System"

---

# 4. Визуальное направление

Основной стиль:

Military + Survival + Sci-Fi.

Смешение:

## Fallout 4

Берём:

- атмосферность;
- Pip-Boy подход;
- RPG элементы;
- характеристики персонажа.

---

## Fallout 76

Берём:

- командный HUD;
- отображение игроков;
- быстрые взаимодействия;
- информацию поверх мира.

Пример:

Во время игры отображаются:

- уровень игрока;
- союзники;
- состояние команды;
- цели;
- оружие;
- боезапас;
- здоровье.

---

## Titanfall 2

Берём:

- чистый боевой HUD;
- военную технологичность;
- связь пилот ↔ Titan;
- быстрый доступ к информации.

---

## State of Decay 2

Берём:

- управление базой;
- ресурсы;
- подготовку экспедиций;
- долгосрочное развитие.

---

# 5. Main Menu System

Главное меню является первым экраном игрока.

Стиль:

- военный терминал;
- система управления бункером;
- технологический интерфейс.

---

Структура:


BUNKER PROTOCOL

[ CONTINUE ]

[ NEW GAME ]

[ LOAD GAME ]

[ CHARACTER ]

[ AIMP ]

[ SETTINGS ]

[ CREDITS ]

[ EXIT ]


---

## Continue

Продолжить последнюю игру.

Показывает:

- последний персонаж;
- последний AIMP;
- последнюю локацию;
- состояние мира.

Пример:


LAST SESSION

Character:
Survivor-01

Location:
Northern Sector

AIMP:
Active

Titan:
BT-72 Ready


---

# 6. Profile System

Игрок имеет профиль.

Профиль хранит:

- персонажей;
- прогресс;
- статистику;
- достижения;
- историю экспедиций.

---

Пример:


PLAYER PROFILE

Name:
Robert

Level:
25

Expeditions:
64

AIMP:
Northern Base

Titan:
BT-72


---

# 7. Character Creation System

Создание персонажа является важным этапом.

Вдохновение:

Fallout 4.

---

Структура:


CHARACTER CREATION

Identity
Appearance
Background
Attributes
Skills
Confirm

---

# 8. Identity

Настройки:

- имя;
- голос;
- возраст;
- базовые данные.

---

# 9. Appearance

Настройки:

- лицо;
- волосы;
- тело;
- детали внешности;
- одежда.

---

# 10. Background System

Предыстория персонажа.

Примеры:

## Engineer

Преимущества:

- ремонт;
- технологии;
- Titan upgrades.

Недостатки:

- слабее в прямом бою.

---

## Soldier

Преимущества:

- оружие;
- броня;
- бой.

Недостатки:

- меньше технических возможностей.

---

## Explorer

Преимущества:

- поиск;
- разведка;
- исследование.

Недостатки:

- меньше боевых бонусов.

---

# 11. Character Menu

Главное меню персонажа.

Структура:


CHARACTER

├── STATUS

├── ATTRIBUTES

├── SKILLS

├── PERKS

├── EQUIPMENT

├── INVENTORY

├── STATISTICS

└── BIOGRAPHY


---

# 12. Status Screen

Показывает:

- здоровье;
- состояние;
- травмы;
- эффекты;
- голод;
- усталость.

---

Пример:


STATUS

HP:
85%

Energy:
70%

Condition:
Stable

Radiation:
0%


---

# 13. Skills Menu

Система развития персонажа.

Основа:

Fallout Perks + RPG Tree.

---

Пример:


ENGINEERING

Basic Repair

    ↓

Advanced Repair

    ↓

Titan Modification

    ↓

Industrial Technology


---

Категории:

## Engineering

- ремонт;
- технологии;
- производство;
- Titan.

---

## Combat

- оружие;
- защита;
- тактика.

---

## Survival

- ресурсы;
- исследование;
- выживание.

---

## Science

- исследования;
- терминалы;
- технологии.

---

## Leadership

- AIMP;
- NPC;
- управление.

---

# 14. Main Gameplay HUD

HUD используется постоянно.

Главное правило:

Интерфейс не должен мешать игре.

---

Структура:


HP

ENERGY

STATUS

      CROSSHAIR

WEAPON

AMMO

BT-72 LINK


---

# 15. Combat HUD

Во время боя отображается:

- здоровье;
- оружие;
- патроны;
- враги;
- союзники;
- цели.

---

Элементы Fallout 76:

- уровни игроков;
- команда;
- маркеры.

---

Элементы Titanfall 2:

- чистота;
- скорость;
- военный стиль.

---

# 16. Pip-Pad System

Главный персональный интерфейс игрока.

Pip-Pad это:

- планшет;
- компьютер;
- карта;
- система управления.

---

Структура:


PIP-PAD

├── STATUS

├── MAP

├── INVENTORY

├── EQUIPMENT

├── MISSIONS

├── RADIO

├── DATABASE

└── SETTINGS


---

# 17. Pip-Pad Map

Особенность:

Мир не открыт сразу.

Начальное состояние:


UNKNOWN AREA


После исследования:


DISCOVERED AREA


---

Карта зависит от:

- разведки;
- вышек;
- экспедиций;
- технологий.

---

# 18. Inventory System

Инвентарь должен быть удобным.

Функции:

- категории;
- поиск;
- фильтры;
- сравнение;
- состояние предметов.

---

Пример:


ASSAULT RIFLE

Damage:
45

Condition:
72%

Weight:
8 KG

Modification:

Scope


---

# 19. Equipment Menu

Управление:

- оружием;
- бронёй;
- инструментами;
- модулями.

---

# 20. Titan Interface

Уникальная система проекта.

Titan:

не транспорт.

Titan:

боевой партнёр и мобильная система игрока.

---

Меню:


TITAN CONTROL

├── STATUS

├── CORE

├── ARMOR

├── WEAPONS

├── ENERGY

├── MODULES

└── DAMAGE REPORT


---

Пример:


BT-72 STATUS

POWER:
82%

HEAT:
34%

DAMAGE:
12%

MODULES:

Scanner

Armor

Storage


---

# 21. AIMP Interface

AIMP является личным лагерем игрока.

Каждый игрок имеет свой AIMP.

---

Меню:


AIMP CONTROL

├── BASE

├── STORAGE

├── WORKSHOP

├── ENERGY

├── DEFENSE

├── RESEARCH

└── EXPANSION


---

# 22. Expedition Planning UI

Перед выходом игрок готовит экспедицию.

---

Пример:


EXPEDITION

LOCATION:

Northern Factory

REQUIRED:

Food

Ammo

Energy

Repair Kit

RISK:

HIGH


---

# 23. Settings Menu

Разделы:

## Graphics

- Resolution;
- FPS;
- Shadows;
- Textures;
- Effects.

---

## Audio

- Music;
- Effects;
- Voice;
- Radio.

---

## Controls

- Keyboard;
- Mouse;
- Controller.

---

## Gameplay

- Difficulty;
- HUD;
- Tutorials;
- Accessibility.

---

# 24. Pause Menu

Во время игры:


PAUSE

RESUME

SAVE

LOAD

CHARACTER

PIP-PAD

AIMP

SETTINGS

EXIT


---

# 25. Loading Screen

Экран загрузки является частью мира.

Показывать:

- состояние оборудования;
- информацию;
- советы;
- события.

---

Пример:


BT-72 SYSTEM CHECK

POWER:

82%

LAST LOCATION:

NORTHERN SECTOR


---

# 26. Multiplayer UI

Модель:

State of Decay 2.

Не MMO.

---

Основные правила:

- каждый игрок создаёт своего персонажа;
- каждый игрок имеет свой AIMP;
- каждый игрок имеет собственную прогрессию;
- Host управляет состоянием мира.

---

# 27. Development Priority

## P0 — обязательно

- Main Menu;
- Pause Menu;
- Settings;
- Basic HUD;
- Save/Load.

---

## P1 — основной игровой интерфейс

- Character Menu;
- Inventory;
- Equipment;
- Pip-Pad.

---

## P2 — расширенные системы

- Skill Tree;
- Perks;
- Titan Interface;
- AIMP Interface.

---

## P3 — дополнительные системы

- Terminal UI;
- NPC Interface;
- Research System;
- Expedition Management.

---

# 28. Что проверить перед использованием документа

Перед тем как считать этот документ финальным, проверить:

## Код

- какой UI framework используется в игре;
- существует ли UI Manager;
- какие окна уже реализованы;
- как работает Input System.

---

## Персонаж

Проверить:

- название Player класса;
- название Titan класса;
- XP систему;
- Level систему;
- Skill систему.

---

## Save System

Проверить:

- что уже сохраняется;
- как расширять сохранения;
- совместимость старых файлов.

---

## Multiplayer

Проверить:

- что хранит Host;
- что хранит Client;
- как синхронизируются игроки.

---

## Камера и управление

Проверить:

- изометрия;
- третье лицо;
- FPS;
- переключение режимов.

# Final Vision

UI Bunker Protocol должен создавать ощущение:


Я выживший.

У меня есть мой персонаж.

У меня есть мой Titan.

У меня есть мой AIMP.

Я исследую неизвестный мир.

Я управляю ресурсами.

Я создаю свою историю.


Главная цель:

Создать не меню игры.

Создать интерфейс выживания внутри мира.

---
