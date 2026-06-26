# INTEGRATION PHASES

Цель: определить пошаговый порядок переноса, исправления и стабилизации `Izometrical_Project` при объединении механик из `GMyGameDoNotTouch` и `Video_Game_Izom`.

---

## Главный принцип

Нельзя переносить всё одновременно.

Если пытаться тащить:
- AI,
- танк,
- сюжет,
- инвентарь,
- погоду,
- сейвы,
- строительство,
- редактор,
- мультиплеер

одним большим куском, проект почти гарантированно станет нерабочим.

Нужна фазовая интеграция.

---

# PHASE 0 — Полный аудит и фиксация источников правды

## Цель
Понять, что именно переносим, откуда и в каком виде.

## Задачи
1. Пройти все ветки трёх репозиториев.
2. Пройти историю коммитов.
3. Прочитать все `.md` и заметки.
4. Собрать список механик.
5. Собрать список багов.
6. Выявить устаревшие и конфликтующие реализации.
7. Назначить source of truth по каждой системе.

## Артефакты
- `MERGE_MASTER_PLAN.md`
- `REPOSITORY_AUDIT_CHECKLIST.md`
- `MECHANICS_TRANSFER_MATRIX.md`
- `TARGET_ARCHITECTURE.md`
- `BUG_RISK_REGISTER.md`
- `SAVE_COMPATIBILITY_PLAN.md`
- `ASSET_PATH_NORMALIZATION.md`

## Критерий завершения
- понятно, откуда брать каждую механику;
- понятно, что переносить, а что переписывать;
- понятен порядок работ;
- известны главные риски.

---

# PHASE 1 — Стабилизация базового каркаса проекта

## Цель
Сделать так, чтобы целевой проект имел стабильный минимум, на который можно безопасно наслаивать остальные системы.

## Задачи
1. Обновить `include/TitanAI.hpp`.
2. Обновить `src/main.cpp`.
3. Исправить пути к ассетам и регистр директорий:
   - `assets/Enemies/` → `assets/enemies/`
   - `assets/Vehicles/` → `assets/vehicles/`
4. Проверить, что проект собирается.
5. Проверить, что проект запускается.
6. Убрать самые грубые конфликтующие legacy-куски.
7. Зафиксировать единый main/update loop.

## Что нельзя делать в этой фазе
- не тащить сразу story;
- не тащить сразу multiplayer;
- не тащить giant HostileAI без разборки;
- не вносить десятки новых механик, пока baseline нестабилен.

## Риски
- новый `TitanAI.hpp` конфликтует со старым main loop;
- новый `main.cpp` рассчитывает на отсутствующие функции/типы;
- asset path fixes ломают старые загрузчики;
- старый код может зависеть от нестандартного регистра путей.

## Критерий завершения
- проект компилируется;
- проект запускается;
- базовые ресурсы загружаются;
- нет критических path-related падений;
- main loop стабилен.

---

# PHASE 2 — Базовая архитектурная декомпозиция

## Цель
Развести по границам самые важные системы, чтобы дальше переносить механику без хаоса.

## Задачи
1. Выделить ядро game state.
2. Выделить player state.
3. Выделить vehicle/titan state.
4. Выделить damage pipeline.
5. Выделить inventory/item baseline.
6. Выделить world interaction baseline.
7. Подготовить save schema с version field.
8. Снизить зависимость gameplay от giant-file логики.

## Что должно появиться
- понятные структуры данных;
- явные update-функции;
- единые точки входа для:
  - урона,
  - действий игрока,
  - посадки/высадки,
  - интеракций,
  - сохранения.

## Риски
- слишком ранний рефакторинг без работающего baseline;
- несоответствие старых struct/enum;
- hidden dependencies из giant runtime файлов.

## Критерий завершения
- есть единый каркас для player/combat/vehicle/world/save;
- новые механики можно встраивать не в хаос, а в понятные слои.

---

# PHASE 3 — Core gameplay first

## Цель
Сделать полноценное игровое ядро до переноса глубокой системности.

## Задачи
1. Довести player movement/state до стабильного состояния.
2. Довести combat baseline.
3. Встроить единый damage pipeline.
4. Встроить mount/dismount.
5. Встроить базовое управление танком/титаном.
6. Встроить инвентарь и ammo baseline.
7. Встроить базовое взаимодействие с миром.
8. Встроить save/load baseline.

## Что входит в minimum playable loop
- игрок двигается;
- может атаковать;
- может получать урон;
- может лечиться/перезаряжаться или как минимум система готова к этому;
- может садиться в технику и выходить;
- техника работает хотя бы на базовом уровне;
- можно сохранять/загружать состояние без полного развала.

## Риски
- AI и vehicle могут требовать уже более сложный damage model;
- сейвы могут сломаться при добавлении новых полей;
- interaction и vehicle mount могут конфликтовать по input/state.

## Критерий завершения
- есть рабочий core loop игры;
- игра уже не просто технодемо;
- можно переходить к врагам и расширенным системам.

---

# PHASE 4 — Hostile AI и боевая экосистема

## Цель
Вернуть полноценную боевую глубину.

## Задачи
1. Перенести HostileAI framework.
2. Перенести Awareness / Aggro.
3. Перенести типы врагов:
   - VerminRush
   - GhoulRush
   - HumanTactical
   - RobotControl
4. Перенести attack cooldowns.
5. Перенести damaged/disabled states.
6. Перенести mechanical hostile damage.
7. Проверить совместимость AI с player-on-foot и player-in-vehicle.
8. Проверить combat vs shock/explosion behavior.

## Важно
HostileAI почти наверняка нельзя копировать giant блоком.
Нужно:
- декомпозировать;
- выделить данные;
- выделить state machine;
- адаптировать к новой update-архитектуре.

## Риски
- AI логика зависит от старого world layout;
- AI perception завязана на старые distance checks;
- robot damage может конфликтовать с новым damage pipeline.

## Критерий завершения
- враги стабильно спавнятся;
- враги видят игрока корректно;
- враги атакуют корректно;
- урон и отключение частей работает;
- AI не ломается при технике.

---

# PHASE 5 — Survival, status и environment systems

## Цель
Добавить атмосферу и системную глубину выживания.

## Задачи
1. Перенести stress system.
2. Перенести SecondWind.
3. Перенести SoulLine.
4. Перенести heal/reload actions полностью.
5. Перенести rations и баффы.
6. Перенести EtherFog.
7. Перенести AcidRain.
8. Перенести visibility modifiers.
9. Перенести environmental damage.
10. Перенести shock wave.

## Важно
Эти системы должны опираться на уже готовые:
- DamageSystem
- StatusEffectSystem
- PlayerState
- AI perception hooks
- WeatherState

## Риски
- слишком ранняя интеграция погодных эффектов может ломать AI;
- критические survival-состояния могут конфликтовать с vehicle mode;
- shock wave может ломать aim/controller state.

## Критерий завершения
- выживание стало системным;
- погода влияет и на игру, и на восприятие;
- статусы и критические состояния работают предсказуемо.

---

# PHASE 6 — Vehicle depth expansion

## Цель
Сделать технику не базовой болванкой, а полноценной системой.

## Задачи
1. Перенести seat swap.
2. Перенести driver/gunner role split.
3. Перенести Bucket Rig.
4. Перенести Ram Shield.
5. Перенести Tow Coupler.
6. Перенести thermal load.
7. Перенести cockpit combat hooks.
8. Перенести damage by subsystems.
9. Перенести repair usage.
10. Подготовить hooks для hangar.

## Риски
- seat state machine может сломать input;
- utility modules могут требовать item/module schema;
- thermal load может конфликтовать с weapon fire loop.

## Критерий завершения
- техника имеет роли, модули и ограничения;
- повреждение техники ощущается системно;
- механики техники встроены в основной gameplay loop.

---

# PHASE 7 — World progression, terminals, story events

## Цель
Перенести структурированный прогресс игры и сценарные зоны.

## Задачи
1. Перенести terminal interaction.
2. Перенести terminal sync / pip-pad sync.
3. Перенести zone triggers.
4. Перенести:
   - CryoLocker
   - Archive
   - Garage
   - Exterior
   - FirstCombat
   - Return
5. Перенести BT-72 recovery route.
6. Перенести clearance progression.
7. Перенести surface progression.
8. Встроить сохранение story flags.

## Важно
Сюжет нельзя держать в виде разрозненных `if` по нескольким cpp.

Нужен единый:
- StoryEventSystem
- StoryFlags registry
- trigger pipeline

## Риски
- story зависит от world state, vehicle state и terminal state сразу;
- старые скриптовые события могут предполагать жёсткий порядок действий;
- сейвы могут ломаться, если флаги не сериализуются.

## Критерий завершения
- сюжетные зоны работают;
- прогресс не ломается между загрузками;
- события запускаются один раз и в правильной последовательности.

---

# PHASE 8 — Inventory depth, loot, skills, pip-pad

## Цель
Добавить слой RPG/системной прогрессии и контентной глубины.

## Задачи
1. Перенести loot generator.
2. Перенести loot tables по редкости.
3. Перенести service kits.
4. Перенести module items.
5. Перенести tape items.
6. Перенести skill framework.
7. Перенести:
   - ArchiveSync
   - FootKill
   - TankAction
   - StressSurvival
8. Перенести pip-pad.
9. Перенести themes/display modes.
10. Перенести radio/tape playback hooks.

## Риски
- skills могут влезать в combat/survival/tank одновременно;
- pip-pad может быть смешан со старым UI кодом;
- item schema может оказаться недостаточной.

## Критерий завершения
- предметы, лут, навыки и интерфейс прогрессии работают совместно;
- нет дублирующих item-definition систем;
- tape/radio и pip-pad встроены логично.

---

# PHASE 9 — Breakables, prefab/content systems, hangar

## Цель
Добавить контентную и мирную системную глубину.

## Задачи
1. Перенести reactive breakables framework.
2. Перенести glass/vegetation/crate reactions.
3. Перенести prefab library.
4. Перенести hangar repair/modification.
5. Перенести service workflows в ангаре.
6. Проверить world persistence для разрушений и модификаций.

## Риски
- breakables могут быть тяжёлыми по производительности;
- prefab library может зависеть от старого world editor format;
- hangar сильно связан с vehicle module schema.

## Критерий завершения
- мир реагирует на действия игрока;
- ангар работает как полноценный сервисный узел;
- контентные системы не ломают сейвы.

---

# PHASE 10 — Building / ToolGun / Editor systems

## Цель
Вернуть инструменты строительства и world authoring.

## Задачи
1. Довести C.A.M.P. build system.
2. Перенести wall placement.
3. Перенести turret placement.
4. Перенести crate placement.
5. Перенести snap logic.
6. Перенести validation.
7. Перенести ToolGun.
8. Перенести World Editor.
9. Перенести undo/redo.
10. Перенести export pipeline.

## Важно
Это должно быть отделено от основного production gameplay.

## Риски
- editor/runtime код смешан;
- construction objects не сериализуются правильно;
- toolgun может ломать game balance/state.

## Критерий завершения
- строить можно стабильно;
- dev-tools работают отдельно и не ломают обычный gameplay;
- editor-функции управляемы.

---

# PHASE 11 — Save/Profile/Session hardening

## Цель
Сделать устойчивую систему сохранений для уже насыщенного проекта.

## Задачи
1. Завершить profile data.
2. Завершить session state.
3. Внедрить format versioning.
4. Внедрить migration path.
5. Добавить safe defaults для новых полей.
6. Проверить backward compatibility по возможности.
7. Прогнать сценарии:
   - save/load in combat
   - save/load in vehicle
   - save/load after story event
   - save/load after weather state change
   - save/load after build placement
   - save/load after breakable destruction

## Риски
- накопленные системы раскрывают старые дефекты сериализации;
- разные подсистемы забывают сохранять свои состояния.

## Критерий завершения
- сохранения стабильны;
- новые поля не ломают старые загрузки;
- world/player/story/vehicle state восстанавливаются предсказуемо.

---

# PHASE 12 — Optimization, asset hardening, polish

## Цель
Довести проект до устойчивого и приятного состояния.

## Задачи
1. Перенести/оценить SpatialGrid.
2. Перенести/доделать OBJ loading.
3. Нормализовать asset pipeline.
4. Убрать битые ресурсы.
5. Добавить fallback assets.
6. Убрать дубли legacy-кода.
7. Оптимизировать update hotspots.
8. Оптимизировать AI/world queries.
9. Проверить memory/perf risks.
10. Сделать regression pass.

## Критерий завершения
- проект стабилен;
- ресурсы грузятся надёжно;
- производительность приемлема;
- кодовая база чище, чем в исходных репозиториях.

---

# PHASE 13 — Multiplayer / network (только если всё остальное стабильно)

## Цель
Подключить сетевой слой без разрушения single-player архитектуры.

## Задачи
1. Разобрать Winsock baseline.
2. Выделить networking abstraction.
3. Подготовить replicated/session entities.
4. Перенести lobby/chat/voice при необходимости.
5. Ограничить область сетевой интеграции.
6. Не ломать single-player path.

## Почему это последняя фаза
Если делать это раньше:
- усложнится every-system integration;
- возрастёт число трудноуловимых багов;
- скорость прогресса сильно упадёт.

## Критерий завершения
- сеть подключена как отдельный слой;
- single-player не деградировал;
- multiplayer не основан на хаотичных костылях.

---

# Cross-phase rules

## Правило 1
После каждой фазы нужен regression pass.

## Правило 2
Нельзя переносить новую большую систему без проверки:
- compile
- run
- baseline interaction
- basic save/load

## Правило 3
Если giant legacy block слишком грязный:
- сначала декомпозировать,
- потом интегрировать.

## Правило 4
Если система требует ещё неготовой архитектуры:
- не форсить,
- отложить в следующую фазу.

## Правило 5
Каждая механика должна иметь:
- source of truth;
- owner system;
- save strategy;
- test criterion.

---

# Минимальный маршрут к playable milestone

Если нужен краткий путь к первой действительно рабочей версии:

1. Phase 1
2. Phase 2
3. Phase 3
4. Phase 4
5. часть Phase 5
6. часть Phase 6
7. базовый save hardening

Это даст:
- движение,
- бой,
- врагов,
- технику,
- базовое выживание,
- базовую системность.

---

# Что считать major milestones

## Milestone A — Stable Baseline
После Phase 1-2  
Проект стабилен, собирается, архитектурно не разваливается.

## Milestone B — Playable Core
После Phase 3-4  
Есть реально играбельная основа.

## Milestone C — Systemic Game
После Phase 5-8  
Игра начинает быть глубокой и цельной.

## Milestone D — Content-Complete Runtime
После Phase 9-12  
Большинство механик перенесено и отполировано.

## Milestone E — Extended Features
После Phase 13  
Сеть и поздние расширения.

---

# Следующие файлы

После этого логично сделать ещё минимум 3 обязательных файла:

1. `BUG_RISK_REGISTER.md`
2. `SAVE_COMPATIBILITY_PLAN.md`
3. `ASSET_PATH_NORMALIZATION.md`

Они нужны, чтобы не потерять критические проблемы при реальной интеграции.