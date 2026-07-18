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

---

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
* [ ] Manual QA: feel-test player slide, wall blocking, and 3D camera movement in the running game.

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

---

# СЛЕДУЮЩИЙ ШАГ

Начинать с:

1. `CollisionSystem.cpp`
2. `InteractionManager.cpp`
3. `LootManager.cpp`
4. `DoorTransition.cpp`
5. `SaveGame.cpp`

После этих пяти задач проект перейдёт из стадии «3D-реконструкция движка» в стадию «реально играбельный Bunker Protocol».

---

# CODEX — РАБОЧЕЕ ПРОДОЛЖЕНИЕ

Этот блок добавлен как рабочее продолжение roadmap. Существующие разделы выше не удалять:
они остаются главным планом проекта.

## Правило продолжения

1. P0, P1 и P2 из этого файла остаются базовым roadmap.
2. Новые задачи добавлять ниже, не переписывая уже существующие разделы.
3. Старые runtime-проверки из `Bunker_Protocol_Vision.md` оставить в конце как финальный QA-чеклист.
4. Если задача из старого рабочего списка пересекается с новым roadmap, считать новый roadmap главным источником, а старую задачу переносить в финальный QA или в соответствующий P-раздел.

## Следующий рабочий фокус

После закрытия P0 из нового roadmap переходить к P1 и P2 как к вертикальному срезу.
После этого рабочее развитие продолжается с P3:

1. P3 — уникальные механики Bunker Protocol.
2. P4 — NPC, фракции и экономика.
3. P5 — кооператив.
4. P6 — оптимизация и AA-уровень.

---

# ФИНАЛЬНЫЙ QA-ЧЕКЛИСТ

Эти пункты не удалять. Они остаются в конце файла и проверяются после крупных изменений 3D/render/ECS/save-load.

- [ ] Камера двигается/следит за игроком в 3D.
- [ ] Mouse aim/toolgun/CAMP placement используют правильную world position.
- [ ] HUD/PipPad/Terminal снова видны поверх 3D.
- [ ] Ground, terrain и cube имеют разные material colors.
- [ ] Save/load сохраняет не только старый 2D state, но и новые 3D/ECS данные.
