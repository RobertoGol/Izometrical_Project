# MERGE EXECUTION CHECKLIST

Цель: дать практический пошаговый чеклист выполнения слияния механик и исправлений в `Izometrical_Project`, чтобы работа шла без хаоса и без потери контроля над проектом.

---

## Главный принцип

Каждый большой шаг должен проходить по одной и той же дисциплине:

1. понять источник;
2. понять зависимости;
3. подготовить замену;
4. внести изменение;
5. проверить compile;
6. проверить run;
7. проверить smoke test;
8. зафиксировать результат;
9. только потом двигаться дальше.

---

# 1. Подготовительный чеклист перед любым переносом

## PRE-001 — Определить механику или файл
- Что именно переносится?
- Это:
  - файл,
  - часть файла,
  - система,
  - конфиг,
  - ассеты,
  - сериализация,
  - UI,
  - story logic?

## PRE-002 — Назначить source of truth
- из какого репозитория?
- из какой ветки?
- из какого коммита?
- это точно лучшая версия, а не устаревшая?

## PRE-003 — Зафиксировать owner-system
- кому принадлежит эта логика?
- `PlayerSystem`?
- `DamageSystem`?
- `AISystem`?
- `VehicleSeatSystem`?
- `StoryEventSystem`?
- `SaveSystem`?

## PRE-004 — Выписать зависимости
- какие include нужны?
- какие struct/enum нужны?
- какие assets/data нужны?
- какие другие системы должны уже существовать?

## PRE-005 — Оценить тип работы
- `PORT`
- `ADAPT`
- `REWRITE`
- `DECOMPOSE`
- `BUGFIX`

---

# 2. Чеклист первой критической волны

## W1-001 — Подготовить сравнение `TitanAI.hpp`
- открыть текущий `include/TitanAI.hpp`
- открыть новую версию
- выписать:
  - новые методы
  - новые поля
  - изменённые сигнатуры
  - новые зависимости
  - устаревшие части

## W1-002 — Подготовить сравнение `src/main.cpp`
- открыть текущий `src/main.cpp`
- открыть новую версию
- разбить оба файла на:
  - includes
  - globals/state
  - init
  - input
  - update
  - render
  - shutdown
- выписать различия

## W1-003 — Подготовить список compile-break risks
Особенно для:
- `TitanAI.hpp`
- `main.cpp`
- asset path changes

## W1-004 — Подготовить список run-break risks
- missing assets
- missing init order
- state mismatch
- null-like references
- old API usage

---

# 3. Чеклист по asset normalization

## AS-001 — Переименовать директории
- `assets/Enemies/` → `assets/enemies/`
- `assets/Vehicles/` → `assets/vehicles/`

## AS-002 — Проверить регистр файлов внутри
- `.png` vs `.PNG`
- `.obj` vs `.OBJ`
- inconsistent filename case

## AS-003 — Просканировать код
Искать:
- `Enemies/`
- `Vehicles/`
- `Assets/`
- `\\`
- mixed-case path strings

## AS-004 — Просканировать data/config
Проверить:
- json/txt/md/config references
- prefab references
- loot/story/skill/weather references
- model path references

## AS-005 — Добавить missing asset logging
Даже если пока без полноценного resolver.

## AS-006 — Проверить базовый запуск после path-fix
- enemy assets грузятся
- vehicle assets грузятся
- ui assets грузятся
- audio assets грузятся

---

# 4. Чеклист controlled merge для `TitanAI.hpp`

## TA-001 — Скопировать/внести новую структуру в staging-варианте
Не делать слепую замену без понимания diff.

## TA-002 — Проверить зависимости заголовка
- какие forward declarations нужны?
- какие include нужны?
- появились ли новые типы?

## TA-003 — Проверить все usage sites
Найти:
- создание объекта
- вызовы методов
- доступ к полям
- callbacks/hooks

## TA-004 — Проверить новые возможности
- `tryMount`
- `dismount`
- `fireFromCockpit`
- stress hooks
- core damage / system damage logic

## TA-005 — Устранить compile errors
Только после этого двигаться дальше.

## TA-006 — Прогнать baseline run
Проверить:
- запуск;
- отсутствие мгновенного крэша;
- отсутствие очевидного рассинхрона состояний.

---

# 5. Чеклист controlled merge для `src/main.cpp`

## MC-001 — Не затирать blindly
Сначала:
- сравнение секций;
- mapping old responsibilities → new responsibilities.

## MC-002 — Проверить init order
- assets
- player state
- world state
- AI/titan init
- UI init
- generators/helpers

## MC-003 — Проверить update order
- input
- player
- combat
- damage
- AI
- vehicle
- world interactions
- story
- UI

## MC-004 — Проверить render order
- world
- entities
- effects
- HUD
- terminal/pip-pad
- debug overlays

## MC-005 — Проверить hooks
- `TimeShift`
- `TextureGenerator`
- `TitanAI`
- asset loading
- any special update bridges

## MC-006 — Убедиться, что логика не потерялась
Особенно:
- input gating
- state transitions
- update calls
- cleanup calls

## MC-007 — Compile + run + smoke test
После merge:
- compile
- launch
- basic movement
- basic render
- no immediate missing-resource catastrophe

---

# 6. Чеклист по damage unification

## DM-001 — Найти все текущие пути урона
- player damage
- enemy damage
- vehicle damage
- explosion damage
- environmental damage
- stress-triggered reactions

## DM-002 — Определить обходные legacy paths
Найти места, где HP/parts меняются напрямую.

## DM-003 — Выделить единый damage entry point
- damage request
- validation
- mitigation
- application
- reactions
- death/disable

## DM-004 — Проверить integration points
- CombatSystem
- AISystem
- VehicleDamageSystem
- SurvivalSystem
- EnvironmentSystem

## DM-005 — Прогнать сценарии
- player takes direct hit
- enemy takes direct hit
- explosion near player
- explosion near hostile
- vehicle subsystem damage
- environmental tick damage

---

# 7. Чеклист по player/vehicle state integration

## PV-001 — Зафиксировать минимальные player states
- OnFoot
- Mounting
- MountedDriver
- MountedGunner
- Dismounting
- Reloading
- Healing
- Interacting
- Critical/Dead

## PV-002 — Зафиксировать seat ownership truth
- кто сидит
- где сидит
- может ли стрелять
- может ли вести
- может ли выйти

## PV-003 — Проверить transition rules
- on foot → mount
- mount → seated
- driver ↔ gunner
- seated → dismount
- mounted + reload/heal restrictions

## PV-004 — Проверить save/load consequences
- mounted state
- seat state
- vehicle link consistency

---

# 8. Чеклист по AI migration

## AI-CHK-001 — Выделить perception отдельно
- range
- LOS
- weather visibility
- target type

## AI-CHK-002 — Выделить awareness/aggro отдельно
- detect
- build awareness
- engage
- lose target
- reacquire

## AI-CHK-003 — Выделить archetypes отдельно
- VerminRush
- GhoulRush
- HumanTactical
- RobotControl

## AI-CHK-004 — Проверить attack execution
- cooldowns
- melee/ranged
- allowed targets
- reactions to mounted player

## AI-CHK-005 — Проверить damage reactions
- sensor damage
- weapon damage
- mobility damage
- disable/death transitions

## AI-CHK-006 — Smoke tests
- враг замечает игрока
- враг преследует игрока
- враг теряет игрока
- враг реагирует на технику
- робот теряет часть функций при повреждениях

---

# 9. Чеклист по survival/environment

## SV-001 — Проверить stress ownership
- кто копит?
- кто тратит?
- кто триггерит critical transitions?

## SV-002 — Проверить SecondWind / SoulLine
- preconditions
- single-use / repeat rules
- compatibility with damage pipeline

## SV-003 — Проверить heal/reload actions
- start
- interrupt
- complete
- deny states

## SV-004 — Проверить weather
- EtherFog
- AcidRain
- visibility impact
- damage impact

## SV-005 — Проверить shock wave
- aim disruption
- stagger
- stacking behavior

---

# 10. Чеклист по story/terminal integration

## ST-001 — Выделить story flags registry
- один владелец
- сериализуемо
- без разрозненных bool across files

## ST-002 — Проверить terminal interaction pipeline
- interaction detect
- terminal open
- terminal effect
- story sync
- save impact

## ST-003 — Проверить zone events
- preconditions
- trigger
- completion
- one-shot behavior
- save/load consistency

## ST-004 — Проверить сюжетные события
- CryoLocker
- Archive
- Garage
- Exterior
- FirstCombat
- Return

---

# 11. Чеклист по save compatibility

## SA-001 — Ввести `saveFormatVersion`
## SA-002 — Разделить profile/save/session
## SA-003 — Проверить player serialization
## SA-004 — Проверить inventory serialization
## SA-005 — Проверить story flags serialization
## SA-006 — Проверить vehicle serialization
## SA-007 — Проверить breakables/build serialization
## SA-008 — Проверить weather serialization
## SA-009 — Добавить validation on load
## SA-010 — Прогнать save/load matrix

---

# 12. Чеклист regression после каждого большого шага

## RG-001 — Compile regression
- сборка проходит

## RG-002 — Launch regression
- игра запускается

## RG-003 — Asset regression
- базовые ресурсы находятся

## RG-004 — Input regression
- движение не сломано
- базовые действия не сломаны

## RG-005 — Gameplay regression
- бой не сломан
- урон не сломан
- интеракции не сломаны

## RG-006 — Vehicle regression
- mount/dismount работает
- vehicle state не ломается

## RG-007 — Save/load regression
- save/load работает хотя бы на baseline сценариях

## RG-008 — Story sanity regression
- ключевые флаги не ломаются
- очевидных soft lock нет

---

# 13. Что делать, если шаг пошёл плохо

## BAD-001
Остановиться и не тащить следующий крупный блок.

## BAD-002
Определить:
- compile break?
- run break?
- state break?
- asset break?
- save break?

## BAD-003
Откатить только последний рискованный слой мыслимо/логически,
а не пытаться “додавить” ещё 3 системы сверху.

## BAD-004
Обновить risk register и backlog.

## BAD-005
Зафиксировать, какой dependency был недооценён.

---

# 14. Ежедневный рабочий ритм

Рекомендуемый цикл:

1. выбрать одну подзадачу;
2. подтвердить source of truth;
3. внести изменение;
4. compile;
5. run;
6. smoke test;
7. записать результат;
8. только потом брать следующую подзадачу.

---

# 15. Что считать успешным merge execution

Merge execution успешен, если:
- каждый перенос контролируем;
- не накапливаются незамеченные регрессии;
- проект остаётся запускаемым;
- архитектура становится чище, а не грязнее;
- каждая новая система имеет owner и тест-критерий.

---

# Следующий логичный файл

После этого можно делать:

`PLAYTEST_SMOKE_MATRIX.md`

Там будет компактная матрица проверки:
- движение
- бой
- AI
- техника
- погода
- сюжет
- сейвы
- лут
- терминалы
- breakables
- build systems