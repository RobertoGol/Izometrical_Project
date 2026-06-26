# TARGET ARCHITECTURE

Цель: определить конечную архитектуру `Izometrical_Project`, в которую будут безопасно интегрированы механики из `GMyGameDoNotTouch` и `Video_Game_Izom`.

---

## Главный принцип

Нельзя превращать итоговый проект в свалку legacy-кода из трёх репозиториев.

Нужна единая архитектура, где:

- каждая подсистема имеет границы ответственности;
- gameplay не размазан хаотично по `main.cpp`;
- AI не живёт в giant switch на тысячи строк;
- save/load не зависит от случайного порядка полей;
- vehicle/tank логика не ломает пеший gameplay;
- UI не управляет gameplay напрямую;
- editor/debug/network код отделён от core runtime.

---

# 1. Архитектурные цели

1. единый runtime pipeline;
2. единая модель сущностей и состояний;
3. единый damage/status pipeline;
4. единая модель player/vehicle interaction;
5. единый world state;
6. единая сериализация;
7. отдельно gameplay, отдельно render/ui, отдельно tools;
8. возможность поэтапного переноса legacy-механик без постоянных регрессий.

---

# 2. Высокоуровневое разбиение

Проект логически делится на слои:

1. **Core**
2. **Engine Runtime**
3. **World**
4. **Entities**
5. **Gameplay Systems**
6. **AI**
7. **Vehicles**
8. **UI / HUD / Terminal**
9. **Persistence**
10. **Content / Data**
11. **Tools / Debug**
12. **Networking** (поздно, отдельно)

---

# 3. Core

## Ответственность
Базовые низкоуровневые вещи:

- math;
- time;
- ids/handles;
- memory-safe utility wrappers;
- logging;
- assertions;
- config;
- random;
- event queues базового уровня;
- common enums/types.

## Правила
- Core не знает про gameplay.
- Core не знает про конкретные враги, танки и сюжет.
- Core должен быть максимально стабильным.

---

# 4. Engine Runtime

## Ответственность
- запуск приложения;
- init/shutdown;
- input polling;
- frame timing;
- update loop;
- render dispatch;
- audio dispatch;
- asset system bootstrap;
- scene/world bootstrap.

## Правила
- `main.cpp` должен быть тонким.
- В `main.cpp` не должно жить половина игры.
- Главный цикл только вызывает подсистемы в нужном порядке.

## Рекомендуемый порядок кадра

1. input begin/update
2. process OS/system events
3. fixed/semi-fixed gameplay update
4. world simulation update
5. AI update
6. vehicle update
7. interaction/event update
8. UI state update
9. render build
10. audio events flush
11. frame present

---

# 5. World Layer

## Ответственность
- карта;
- зоны;
- триггеры;
- static world objects;
- interactables;
- breakables;
- weather/environment state;
- world query helpers;
- spawn points;
- navigation helpers.

## Что должно здесь жить
- `ZoneTrigger`
- `WorldObject`
- `Interactable`
- `Breakable`
- `EnvironmentState`
- `WeatherState`
- `PrefabInstance`
- world-level flags

## Что не должно здесь жить
- логика UI;
- детали отрисовки HUD;
- сериализация профиля игрока;
- сетевой код.

---

# 6. Entity Layer

## Ответственность
Единая модель сущностей.

Сущности могут быть:
- player;
- hostile;
- npc;
- vehicle/tank/titan;
- projectile;
- loot object;
- breakable object;
- build object;
- scripted proxy entity.

## Минимальные общие поля
- entity id;
- entity type;
- transform/position;
- active/alive flag;
- faction/alignment;
- world membership;
- persistence flags;
- optional gameplay tags.

## Важно
Нельзя держать три несовместимых формата:
- один для player,
- другой для AI,
- третий для vehicle,
если они участвуют в одних и тех же системах урона, взаимодействий и сохранения.

---

# 7. Gameplay Systems Layer

Это набор систем, которые обновляют состояние мира и сущностей.

Основные подсистемы:

1. PlayerSystem
2. CombatSystem
3. DamageSystem
4. InventorySystem
5. InteractionSystem
6. StatusEffectSystem
7. SurvivalSystem
8. LootSystem
9. Story/EventSystem
10. BuildSystem

---

## 7.1. PlayerSystem

### Ответственность
- управление игроком;
- состояния игрока;
- пеший режим;
- посадка/высадка;
- пересадка по местам;
- использование предметов;
- запуск действий heal/reload/interact.

### Рекомендуемые состояния
- OnFoot
- Mounting
- MountedDriver
- MountedGunner
- Dismounting
- Healing
- Reloading
- Interacting
- Staggered
- Downed / Critical
- Dead

### Важно
PlayerSystem не должен сам рассчитывать весь урон, лут, сюжет и UI.

---

## 7.2. CombatSystem

### Ответственность
- стрельба;
- атаки;
- cooldowns;
- projectile/hitscan logic;
- explosion generation;
- target hit resolution;
- thermal load hooks;
- cockpit fire hooks.

### Не должен делать напрямую
- финальное изменение HP по всем правилам;
- сохранение игры;
- story progression;
- UI rendering.

Это всё либо через DamageSystem, либо через Event/State.

---

## 7.3. DamageSystem

### Ответственность
Единая точка применения урона.

Он должен уметь:
- обычный урон;
- взрывной урон;
- environmental damage;
- part/component damage;
- vehicle subsystem damage;
- stress-related reactions;
- shock effects;
- death/disable transitions.

### Почему это критично
Сейчас самый большой риск — несколько разных источников урона с разной логикой.

### Требование
Любой gameplay-урон должен проходить через единый понятный интерфейс.

---

## 7.4. InventorySystem

### Ответственность
- инвентарь игрока;
- ammo pools;
- item stacks;
- consumables;
- modules;
- service kits;
- tapes;
- loot pickup;
- item use validation.

### Требование
Нужна единая item schema:
- item id;
- item type;
- rarity;
- stackability;
- effect payload;
- references to content/data.

---

## 7.5. InteractionSystem

### Ответственность
- поиск доступных интеракций;
- prompts;
- активация терминалов;
- контейнеры;
- двери;
- панели;
- посадка в технику;
- world actions.

### Важно
InteractionSystem только определяет и запускает действия,
но не должен содержать сам по себе весь сюжет.

---

## 7.6. StatusEffectSystem

### Ответственность
- баффы;
- дебаффы;
- временные эффекты от ration;
- статусы от погоды;
- перегрев;
- shock/stagger;
- специальные состояния.

---

## 7.7. SurvivalSystem

### Ответственность
- stress;
- second wind;
- soul line;
- критические состояния;
- environmental survival hooks.

### Примечание
Можно объединить со StatusEffectSystem, но логически лучше держать отдельно,
если survival получается большим.

---

## 7.8. LootSystem

### Ответственность
- loot tables;
- rarity tiers;
- drop generation;
- crate/hostile/world loot;
- scripted loot rewards.

---

## 7.9. StoryEventSystem

### Ответственность
- zone progression;
- story flags;
- cryolocker/archive/garage/exterior events;
- first combat triggers;
- return routes;
- terminal sync hooks.

### Требование
Никаких скрытых сюжетных флагов, разбросанных по разным cpp.
Нужен единый реестр прогресса и триггеров.

---

## 7.10. BuildSystem

### Ответственность
- C.A.M.P./строительство;
- placement preview;
- validation;
- snapping;
- placement commit;
- spawned build objects.

### Примечание
Сначала можно держать в отключённом/дев-режиме, пока ядро не стабилизировано.

---

# 8. AI Layer

AI должен быть отдельным блоком, а не побочным эффектом world update.

## Основные части
1. Perception
2. Awareness/Aggro
3. Decision State
4. Navigation/Movement Intent
5. Attack Logic
6. Damage Reactions
7. Disabled/Death States

---

## 8.1. Perception
Должна учитывать:
- дистанцию;
- LOS;
- шум/события, если есть;
- visibility modifiers от weather;
- состояние игрока;
- состояние player-in-vehicle.

---

## 8.2. Awareness/Aggro
Отдельная логика от “сразу увидел = сразу атакует”.

Нужно:
- нарастание awareness;
- потеря цели;
- reacquire;
- разные параметры по типам врагов.

---

## 8.3. AI archetypes
Нужно поддержать:
- VerminRush
- GhoulRush
- HumanTactical
- RobotControl
- Titan-specific logic
- future hostile archetypes

Лучше через:
- enum archetype
- config table
- state data per instance

---

## 8.4. Damage reactions
AI должен реагировать на:
- часть повреждена;
- сенсоры сломаны;
- оружие сломано;
- mobility crippled;
- explosion shock;
- weather hazards.

---

# 9. Vehicle Layer

Vehicle/Tank/Titan — отдельный крупный домен.

## Подсистемы
1. VehicleControlSystem
2. VehicleSeatSystem
3. VehicleDamageSystem
4. VehicleModuleSystem
5. VehicleRepair/HangarSystem
6. CockpitCombatBridge

---

## 9.1. VehicleControlSystem
- движение;
- поворот;
- ускорение;
- ограничения;
- состояние “исправен / поврежден / обездвижен”.

---

## 9.2. VehicleSeatSystem
- mount;
- dismount;
- seat swap;
- driver/gunner role;
- допустимые действия по месту.

---

## 9.3. VehicleDamageSystem
- damage by subsystem;
- armor/hull;
- sensors;
- weapon;
- mobility;
- core state;
- disable thresholds.

---

## 9.4. VehicleModuleSystem
- Bucket Rig
- Ram Shield
- Tow Coupler
- будущие модули

Нужны:
- слоты,
- equipped state,
- activation logic,
- cooldown/resource constraints.

---

## 9.5. VehicleRepair/HangarSystem
- ремонт;
- обслуживание;
- сервисные комплекты;
- модификации;
- состояние в ангаре.

---

## 9.6. CockpitCombatBridge
Связывает:
- weapon input,
- HUD,
- firing logic,
- thermal load,
- fireFromCockpit,
- part failures.

---

# 10. UI / HUD / Terminal Layer

## Подслои
1. HUD
2. Menus
3. Inventory UI
4. Vehicle UI
5. Terminal UI
6. Pip-Pad UI
7. Story/log/radio feedback

---

## Правила
- UI читает gameplay state, но не владеет им.
- UI может отправлять команды в systems layer.
- Нельзя хранить “истину игры” только внутри UI виджетов.

---

# 11. Persistence Layer

## Основные части
1. SaveGame
2. LoadGame
3. ProfileData
4. SessionState
5. Versioning
6. Migration

---

## 11.1. SaveGame
Должно сохранять:
- player state;
- inventory;
- story flags;
- zone state;
- weather;
- vehicle state;
- hostile persistence если требуется;
- build placements;
- breakable states;
- loot state.

---

## 11.2. ProfileData
Мета-данные:
- настройки;
- прогресс профиля;
- разблокировки;
- возможно статистика.

---

## 11.3. SessionState
Текущее прохождение:
- активная зона;
- прогресс миссии;
- временные эффекты;
- runtime flags;
- активный транспорт.

---

## 11.4. Versioning / Migration
Каждый save должен иметь:
- format version;
- optional migration path;
- safe fallback behavior.

Без этого перенос механик почти гарантированно сломает сохранения.

---

# 12. Content / Data Layer

Нужно стремиться вытаскивать данные из hardcode.

## Что желательно хранить как data/config
- hostile archetype tuning;
- weapon tuning;
- loot tables;
- item definitions;
- weather presets;
- skill definitions;
- vehicle modules;
- story event metadata;
- prefab definitions;
- terminal content;
- radio/tape metadata.

---

# 13. Tools / Debug Layer

## Что сюда относится
- debug overlays;
- developer cheats;
- inspector-like views;
- world editor;
- toolgun;
- validation;
- export tools;
- profiling helpers.

## Главное правило
Tools не должны ломать production runtime и не должны быть впаяны в gameplay намертво.

---

# 14. Networking Layer

Это поздний слой.

## Почему отдельно
Потому что multiplayer нельзя просто “долепить” сверху на хаотичный single-player gameplay.

## Когда трогать
Только после стабилизации:
- world state;
- entity state;
- combat;
- save/session;
- deterministic-ish update boundaries.

---

# 15. Рекомендуемая схема модулей/папок

Пример логического деления:

- `src/core/`
- `src/engine/`
- `src/world/`
- `src/entities/`
- `src/gameplay/`
- `src/ai/`
- `src/vehicles/`
- `src/ui/`
- `src/persistence/`
- `src/content/`
- `src/tools/`
- `src/network/`

И заголовки аналогично:
- `include/core/`
- `include/engine/`
- `include/world/`
- `include/entities/`
- `include/gameplay/`
- `include/ai/`
- `include/vehicles/`
- `include/ui/`
- `include/persistence/`
- `include/content/`
- `include/tools/`
- `include/network/`

---

# 16. Обязательные архитектурные инварианты

1. Один понятный game loop.
2. Один понятный damage pipeline.
3. Один понятный save schema.
4. Один понятный player state model.
5. Один понятный vehicle seat model.
6. Один понятный AI perception/aggro path.
7. Один понятный asset path convention.
8. Сюжетные флаги только в одном управляемом месте.
9. Gameplay не должен зависеть от UI-классов.
10. Editor/debug код не должен быть ядром runtime.

---

# 17. Самые опасные legacy-ловушки

## 17.1. Giant main.cpp
Если половина игры живёт в `main.cpp`, перенос новых систем будет ломать старые.

## 17.2. Giant runtime cpp
Если `GameRuntime.cpp` содержит всё сразу, нельзя просто копировать куски без декомпозиции.

## 17.3. Несколько разных структур одного и того же объекта
Например:
- один формат танка в старом коде,
- другой формат в новом,
- третий формат для сейва.

## 17.4. Урон размазан по разным местам
Это создаёт баги:
- враг умер визуально, но не логически;
- техника обездвижена, но едет;
- stress срабатывает не везде;
- healing обходит ограничения.

## 17.5. Сюжетные флаги в if-ах по всему проекту
Нужно централизовать.

---

# 18. Минимальный целевой migration strategy

## Шаг 1
Стабилизировать:
- `main.cpp`
- `TitanAI.hpp`
- asset paths
- build/run baseline

## Шаг 2
Собрать:
- player state
- combat state
- damage state
- vehicle state
- save schema baseline

## Шаг 3
Перенести:
- hostile ai
- awareness
- weather
- heal/reload
- stress
- vehicle modules
- terminal interaction

## Шаг 4
Перенести:
- story zones
- hangar
- loot tables
- skills
- pip-pad
- tape/radio

## Шаг 5
Перенести:
- construction
- editor/toolgun
- multiplayer
- spatial optimization

---

# 19. Что считается успешной архитектурой

Архитектура успешна, если:

- можно добавлять механики без переписывания всего проекта;
- можно тестировать системы изолированно;
- можно сохранять и загружать игру без хаоса;
- player, AI, vehicle и world не конфликтуют;
- legacy-код постепенно заменяется, а не просто наслаивается;
- итоговый проект остаётся понятным и расширяемым.

---

# Следующий файл

После этого файла нужен:

`INTEGRATION_PHASES.md`

В нём будет:
- конкретная дорожная карта внедрения;
- фазы;
- порядок файлов;
- зависимые шаги;
- критерии завершения каждой фазы.