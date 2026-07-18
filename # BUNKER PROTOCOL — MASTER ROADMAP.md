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

### Status

* [x] Added `InteractionManager.hpp/.cpp`.
* [x] Added best-target query with distance checks.
* [x] Added line-of-sight checks through `PhysicsWorld`.
* [x] Routed `E` interaction through `InteractionManager`.
* [x] Connected current working interaction types: `Terminal`, `Container`, `Vehicle`.
* [ ] Highlight rendering hook is API-ready through `highlightedTarget()`, but visual overlay still needs renderer/UI integration.
* [ ] Door, Pickup, CraftingStation and NPC execution need their own systems before they can be enabled.

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


---

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
