# FILE BY FILE MIGRATION PLAN

Цель: определить практический порядок миграции файлов и подсистем из `GMyGameDoNotTouch` и `Video_Game_Izom` в `Izometrical_Project`, чтобы перенос был контролируемым, а не хаотичным.

---

## Главный принцип

Мигрировать нужно не “всё подряд”, а пакетами:

1. сначала определить source file;
2. затем определить зависимые файлы;
3. затем понять, это:
   - прямой перенос,
   - адаптация,
   - переписывание,
   - декомпозиция giant-file legacy;
4. затем встраивать по фазам.

---

# 1. Первая волна: файлы немедленного baseline

## F-001 — `include/TitanAI.hpp`
- Источник: новая версия из актуального набора изменений
- Цель: `Izometrical_Project/include/TitanAI.hpp`
- Приоритет: `P0`
- Действие: `ADAPT`
- Риск: высокий

### Проверить
- новые поля;
- новые методы;
- `tryMount`;
- `dismount`;
- `fireFromCockpit`;
- stress hooks;
- core/system damage hooks;
- зависимости на другие типы.

### Перед заменой
- выписать API diff;
- найти все include/usage sites;
- найти code paths, ожидающие старую структуру.

### После замены
- устранить compile errors;
- проверить вызовы из `main.cpp` и gameplay loop.

---

## F-002 — `src/main.cpp`
- Источник: новая версия из актуального набора изменений
- Цель: `Izometrical_Project/src/main.cpp`
- Приоритет: `P0`
- Действие: `ADAPT`
- Риск: очень высокий

### Проверить
- что удалено;
- что добавлено;
- какие новые include нужны;
- где завязка на `TimeShift`;
- где завязка на `TextureGenerator`;
- как вызывается `TitanAI`;
- какие legacy update blocks исчезли.

### Перед заменой
- разбить старый и новый файл на секции;
- сопоставить:
  - init
  - input
  - update
  - render
  - shutdown

### После замены
- compile pass;
- run pass;
- baseline gameplay smoke test.

---

## F-003 — `assets/Enemies/`
- Источник: текущий целевой репозиторий + переносимые ассеты
- Цель: `assets/enemies/`
- Приоритет: `P0`
- Действие: `BUGFIX`

### Нужно
- переименовать директорию;
- обновить все ссылки;
- проверить регистр файлов внутри.

---

## F-004 — `assets/Vehicles/`
- Источник: текущий целевой репозиторий + переносимые ассеты
- Цель: `assets/vehicles/`
- Приоритет: `P0`
- Действие: `BUGFIX`

### Нужно
- переименовать директорию;
- обновить все ссылки;
- проверить регистр файлов внутри.

---

# 2. Файлы аудита и архитектурной сверки

## F-005 — giant runtime file(s) из `GMyGameDoNotTouch`
- Пример: `GameRuntime.cpp` и любые аналоги
- Приоритет: `P0`
- Действие: `AUDIT + DECOMPOSE`
- Риск: очень высокий

### Нужно извлечь
- hostile ai;
- awareness;
- weather;
- breakables;
- story triggers;
- survival systems;
- tank/vehicle hidden logic;
- terminal interactions;
- loot generation hooks.

### Важно
Не копировать giant-файл в целевой проект.
Сначала:
- карта секций,
- карта зависимостей,
- список механик.

---

## F-006 — AI-related headers/cpps во всех 3 репозиториях
- Приоритет: `P1`
- Действие: `AUDIT`

### Нужно определить
- какие AI-файлы активные;
- какие устаревшие;
- какие содержат archetype-specific logic;
- какие завязаны на старый world API.

---

## F-007 — player controller files
- Приоритет: `P0`
- Действие: `AUDIT + ADAPT`

### Нужно определить
- где базовый movement;
- где interaction;
- где mounting;
- где heal/reload actions;
- где input gating.

---

## F-008 — vehicle/tank/titan files
- Приоритет: `P0`
- Действие: `AUDIT + ADAPT`

### Нужно определить
- control loop;
- seat logic;
- subsystem damage;
- thermal load;
- utility modules;
- hangar hooks.

---

## F-009 — save/load/profile/session files
- Приоритет: `P0`
- Действие: `AUDIT + REWRITE`

### Нужно определить
- текущий формат;
- есть ли version field;
- где story flags;
- где vehicle state;
- где world persistence;
- есть ли profile/session split.

---

# 3. Вторая волна: core gameplay файлы

## F-010 — player state / gameplay state files
- Приоритет: `P1`
- Действие: `ADAPT`

### Нужно
- ввести явные player states;
- связать с vehicle states;
- связать с action states:
  - heal
  - reload
  - interact
  - mount/dismount

---

## F-011 — weapon/combat files
- Приоритет: `P1`
- Действие: `ADAPT`

### Нужно
- единая стрельба;
- ammo flow;
- explosion damage hooks;
- cockpit fire hooks;
- thermal load integration.

---

## F-012 — damage-related files
- Приоритет: `P0`
- Действие: `REWRITE`

### Нужно
Собрать единый DamageSystem из:
- player damage;
- hostile damage;
- vehicle subsystem damage;
- explosion damage;
- environmental damage;
- stress/critical transitions.

---

## F-013 — inventory/item files
- Приоритет: `P1`
- Действие: `REWRITE`

### Нужно
- единая item schema;
- stacks;
- ammo;
- consumables;
- tapes;
- service kits;
- modules.

---

## F-014 — world interaction files
- Приоритет: `P1`
- Действие: `ADAPT`

### Нужно
- prompts;
- object interaction;
- terminal activation;
- mount interaction;
- loot container interaction.

---

# 4. Третья волна: AI migration files

## F-015 — HostileAI extraction target
- Приоритет: `P1`
- Действие: `DECOMPOSE → PORT`

### Нужно выделить в отдельные логические модули
- perception;
- awareness;
- archetype config;
- movement intent;
- attack logic;
- cooldown logic;
- disabled/death reactions.

---

## F-016 — hostile archetype data/config
- Приоритет: `P1`
- Действие: `DATA`

### Нужно оформить отдельно для:
- VerminRush
- GhoulRush
- HumanTactical
- RobotControl

### Желательно вынести
- speed
- awareness radius
- attack radius
- cooldowns
- damage
- special behavior flags

---

## F-017 — mechanical hostile damage implementation files
- Приоритет: `P1`
- Действие: `ADAPT`

### Нужно
- подсистемы врага;
- слом сенсоров;
- слом оружия;
- слом мобильности;
- AI reaction hooks.

---

# 5. Четвёртая волна: survival/environment files

## F-018 — stress/survival files
- Приоритет: `P1`
- Действие: `PORT + ADAPT`

### Нужно
- stress accumulation;
- second wind;
- soul line;
- critical thresholds;
- player state hooks.

---

## F-019 — heal/reload action files
- Приоритет: `P1`
- Действие: `PORT`

### Нужно
- stateful action timing;
- interruption;
- item/ammo integration;
- UI hooks.

---

## F-020 — weather/environment files
- Приоритет: `P1`
- Действие: `PORT`

### Нужно
- EtherFog;
- AcidRain;
- visibility modifiers;
- damage over time hooks;
- AI perception hooks.

---

## F-021 — shock wave / reaction files
- Приоритет: `P1`
- Действие: `PORT`

### Нужно
- blast reaction;
- stagger/aim disruption;
- possible physics impulse hooks.

---

# 6. Пятая волна: vehicle depth files

## F-022 — seat logic files
- Приоритет: `P1`
- Действие: `PORT`

### Нужно
- seat swap;
- driver/gunner roles;
- allowed actions by seat.

---

## F-023 — vehicle subsystem damage files
- Приоритет: `P1`
- Действие: `ADAPT`

### Нужно
- mobility damage;
- sensor damage;
- weapon damage;
- core damage;
- disable thresholds.

---

## F-024 — utility module files
- Приоритет: `P1`
- Действие: `PORT`

### Модули
- Bucket Rig
- Ram Shield
- Tow Coupler

---

## F-025 — hangar-related files
- Приоритет: `P2`
- Действие: `PORT`

### Нужно
- repair;
- service kits;
- modifications;
- module management.

---

# 7. Шестая волна: story / terminal / progression files

## F-026 — terminal files
- Приоритет: `P1`
- Действие: `PORT`

### Нужно
- terminal interaction;
- terminal UI bridge;
- terminal sync hooks.

---

## F-027 — story/zone event files
- Приоритет: `P2`
- Действие: `DECOMPOSE + PORT`

### Нужно
- CryoLocker;
- Archive;
- Garage;
- Exterior;
- FirstCombat;
- Return;
- progression gates.

---

## F-028 — story flags / progression registry files
- Приоритет: `P1`
- Действие: `REWRITE`

### Нужно
- единый реестр;
- единая сериализация;
- явные preconditions/postconditions.

---

# 8. Седьмая волна: loot / skill / UI depth files

## F-029 — loot generator files
- Приоритет: `P2`
- Действие: `PORT`

---

## F-030 — loot table data files
- Приоритет: `P2`
- Действие: `DATA`

---

## F-031 — skill framework files
- Приоритет: `P2`
- Действие: `PORT`

### Навыки
- ArchiveSync
- FootKill
- TankAction
- StressSurvival

---

## F-032 — Pip-Pad files
- Приоритет: `P2`
- Действие: `PORT`

### Нужно
- base UI;
- themes;
- display modes;
- sync hooks.

---

## F-033 — radio/tape files
- Приоритет: `P2`
- Действие: `PORT`

### Нужно
- collectible tapes;
- playback logic;
- ui/log/subtitle hooks.

---

# 9. Восьмая волна: breakables / prefabs / build files

## F-034 — breakables files
- Приоритет: `P2`
- Действие: `PORT`

### Нужно
- glass;
- vegetation;
- crates;
- gameplay state persistence.

---

## F-035 — prefab library files
- Приоритет: `P2`
- Действие: `PORT`

---

## F-036 — build/C.A.M.P. files
- Приоритет: `P3`
- Действие: `ADAPT`

### Нужно
- walls;
- turrets;
- crates;
- placement;
- validation;
- snapping.

---

## F-037 — ToolGun files
- Приоритет: `P3`
- Действие: `PORT`

---

## F-038 — World Editor files
- Приоритет: `P3`
- Действие: `DECOMPOSE`

### Нужно
- undo/redo;
- validation;
- export;
- semantic authoring if still valuable.

---

# 10. Девятая волна: optimization / assets / network files

## F-039 — SpatialGrid files
- Приоритет: `P3`
- Действие: `AUDIT → DEFER/PORT`

---

## F-040 — OBJ loading files
- Приоритет: `P3`
- Действие: `PORT`

---

## F-041 — asset resolver / path normalization files
- Приоритет: `P0`
- Действие: `REWRITE`

### Нужно
- один helper/resolver;
- path normalization;
- missing asset logging;
- fallback assets.

---

## F-042 — networking files
- Приоритет: `P3`
- Действие: `DEFER`

### Нужно позже оценить
- winsock baseline;
- lobby;
- chat;
- voice;
- session services.

---

# 11. Обязательная карточка для каждого файла

Для каждого реально мигрируемого файла использовать такую запись:

## [File ID] Путь
- Source Repo:
- Source Branch:
- Source Commit:
- Target Path:
- Purpose:
- Depends On:
- Used By:
- Action:
- Priority:
- Risks:
- Pre-Migration Checklist:
- Post-Migration Checklist:
- Save Impact:
- Asset Impact:
- Notes:

---

# 12. Порядок практического применения плана

## Шаг 1
Пройти файлы первой волны:
- `F-001`
- `F-002`
- `F-003`
- `F-004`
- `F-041`

## Шаг 2
Сделать compile/run baseline.

## Шаг 3
Пройти архитектурно-критичные файлы:
- `F-005`
- `F-007`
- `F-008`
- `F-009`
- `F-012`
- `F-013`

## Шаг 4
Встроить core gameplay files.

## Шаг 5
Встраивать AI, survival, vehicle depth, story, loot, build по фазам.

---

# 13. Что считать плохой миграцией файла

Плохая миграция — если файл:
- просто перетянут копипастой;
- сломал compile;
- сломал run;
- принёс скрытые legacy-зависимости;
- дублировал уже существующую систему;
- не имеет owner/responsibility в новой архитектуре.

---

# 14. Что считать хорошей миграцией файла

Хорошая миграция — если:
- понятен источник;
- понятны зависимости;
- понятно, зачем файл нужен;
- он встроен в owner-system;
- после него проект не стал менее управляемым;
- regression checks пройдены.

---

# Следующий файл

После этого логично сделать:

`SYSTEM_OWNERSHIP_MAP.md`

В нём нужно зафиксировать:
- какая подсистема чем владеет;
- кто отвечает за player/ai/vehicle/damage/story/save/assets;
- чтобы при интеграции не было дублирующих центров логики.