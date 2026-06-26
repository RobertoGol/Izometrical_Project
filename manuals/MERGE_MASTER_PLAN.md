# MERGE MASTER PLAN — Izometrical_Project

Цель: полностью перенести, объединить, исправить и довести до рабочего состояния все механики из:

- GMyGameDoNotTouch
- Video_Game_Izom

в целевой репозиторий:

- Izometrical_Project

---

## Главная цель

Сделать максимально качественную, рабочую, цельную игру на рукописном движке, без Unity/Godot/других готовых движков.

---

## Основной принцип работы

Не просто копировать код, а:

1. перечитать все репозитории;
2. прочитать все ветки;
3. прочитать все коммиты;
4. прочитать все `.md` и документацию;
5. извлечь все механики;
6. выявить баги и архитектурные проблемы;
7. спроектировать целевую архитектуру;
8. перенести механики в Izometrical_Project;
9. устранить конфликты между системами;
10. довести проект до стабильной рабочей версии.

---

## Источники

### 1. GMyGameDoNotTouch
Ключевой источник тяжёлых gameplay-механик, боевых систем, AI, сюжетных и системных механик.

### 2. Video_Game_Izom
Источник изометрического геймплея, строительных систем, части UI/редактора/сетевых и world-систем.

### 3. Izometrical_Project
Целевой репозиторий, куда должны быть перенесены и собраны все рабочие механики.

---

## Уже замеченные проблемы

- баги в исходных репозиториях;
- несогласованность архитектуры между проектами;
- старые версии файлов в целевом репозитории;
- проблемы с регистрами путей:
  - `assets/Enemies/` vs `assets/enemies/`
  - `assets/Vehicles/` vs `assets/vehicles/`
- часть механик перенесена не полностью;
- часть систем вообще отсутствует;
- возможны битые зависимости между gameplay, render, asset-loading и input;
- возможны рассинхроны по enum/struct/data layouts;
- возможны сломанные include paths;
- возможны регрессии после частичного переноса TitanAI / main.cpp;
- возможно, некоторые механики сидят не в очевидных файлах, а размазаны по большим runtime-модулям.

---

## Уже подтверждённые пропущенные механики

# Из GMyGameDoNotTouch

1. HostileAI система
   - VerminRush
   - GhoulRush
   - HumanTactical
   - RobotControl

2. Awareness / Aggro система

3. Mechanical Hostile Damage
   - урон по частям
   - сенсоры
   - оружие
   - мобильность

4. Weather система
   - EtherFog
   - AcidRain

5. Radio / Tape система
   - радиоперехваты
   - кассеты / плёнки

6. Rations
   - временные баффы

7. Heal / Reload
   - лечение
   - перезарядка
   - анимационные/состоянийные переходы

8. Tank utility modules
   - Bucket Rig
   - Ram Shield
   - Tow Coupler

9. Seat Swap
   - водитель ↔ стрелок

10. Hangar System
   - ремонт
   - модификации
   - сервис-комплекты

11. Terminal Sync
   - терминалы
   - синхронизация Pip-Pad

12. Zone Events
   - CryoLocker
   - Archive
   - Garage
   - Exterior
   - FirstCombat
   - Return

13. Stress / SecondWind / SoulLine

14. Reactive Breakables
   - стекло
   - растительность
   - ящики
   - физическая реакция

15. Shock Wave
   - волна от взрывов
   - сбитие прицела / реакции

16. Tank Thermal Load
   - перегрев
   - влияние на скорострельность

17. Lanline Services
   - лобби
   - чат
   - голос
   - доставки

18. Pip-Pad customization
   - темы
   - режимы отображения
   - носимые устройства

19. Skill System
   - ArchiveSync
   - FootKill
   - TankAction
   - StressSurvival

20. Story Route
   - восстановление BT-72
   - Clearance
   - Surface progression

21. World Editor
   - undo/redo
   - semantic authoring
   - validation
   - export

22. Loot Generator
   - tier tables
   - Common → Legendary

23. Prefab Library

24. Profile / Session
   - профиль игрока
   - миграция версий

---

# Из Video_Game_Izom

1. LootSystem / C.A.M.P. строительство
   - стены
   - турели
   - ящики
   - частично отсутствует

2. ToolGun
   - редактор карты в runtime

3. Network (Winsock)
   - мультиплеер до 20 игроков

4. SpatialGrid
   - оптимизация коллизий / поиска

5. OBJ Model Loading
   - загрузка 3D моделей `.obj`

---

## Критически важные файлы, которые уже упоминались

- `include/TitanAI.hpp`
- `src/main.cpp`

Также отдельно нужно проверить:

- все AI-related headers/cpps;
- player controller;
- tank/vehicle systems;
- combat systems;
- world runtime;
- UI / HUD / terminal systems;
- resource loading;
- save/profile/session code;
- editor/runtime tools;
- data tables / loot / prefabs;
- assets paths и регистры директорий.

---

## Что нужно сделать по процессу

### Фаза 1. Полный аудит
Нужно:

- пройти все ветки трёх репозиториев;
- пройти историю коммитов;
- собрать список всех крупных систем;
- собрать список зависимостей между ними;
- выявить дубли, конфликты и устаревшие реализации;
- составить карту файлов:
  - откуда переносить,
  - куда переносить,
  - что переписать заново.

Результат фазы:
- полный технический аудит;
- таблица переноса систем;
- архитектурный план интеграции.

---

### Фаза 2. Базовая архитектура целевого проекта
Нужно определить и стабилизировать:

- core loop;
- game state;
- entity lifecycle;
- AI update pipeline;
- combat pipeline;
- vehicle pipeline;
- world interaction pipeline;
- save/load/profile/session;
- asset loading;
- event scripting;
- debug/editor/runtime tools.

Результат:
- единая архитектура без конфликтующих старых кусков.

---

### Фаза 3. Перенос core gameplay
В первую очередь переносится то, без чего игра не является игрой:

1. Player controller
2. Weapon/combat
3. Enemy AI
4. Vehicle/Titan/Tank systems
5. Damage model
6. Interaction
7. World simulation
8. Inventory/loot
9. Save/profile/session
10. UI/HUD

---

### Фаза 4. Перенос продвинутых систем
После базового ядра:

- weather;
- stress/survival;
- story triggers;
- terminal sync;
- hangar;
- seat swap;
- thermal load;
- breakables;
- shock wave;
- radio/tape;
- skills;
- pip-pad;
- prefab/world editor;
- toolgun;
- construction;
- multiplayer;
- spatial optimization;
- obj loading.

---

### Фаза 5. Полировка и стабилизация
Нужно:

- устранить баги;
- стабилизировать ассеты;
- починить все include paths;
- унифицировать naming;
- убрать дубли;
- проверить производительность;
- сделать debug tools;
- провести regression pass.

---

## Приоритет переноса механик

### P0 — блокирующие игру
- main loop consistency
- player controller
- combat
- hostile AI
- damage pipeline
- interaction
- inventory / loot baseline
- save/load
- vehicle/titan baseline
- asset path fixes

### P1 — сильно влияет на игровой процесс
- awareness/aggro
- stress/second wind/soulline
- heal/reload
- tank utility modules
- seat swap
- thermal load
- weather
- breakables
- shock wave

### P2 — системная глубина
- hangar
- terminal sync
- zone events
- skills
- pip-pad
- radio/tape
- profile/session migration
- loot tiers
- prefab library

### P3 — инструменты / расширение
- toolgun
- world editor
- camp/build system
- spatial grid
- obj loading
- multiplayer

---

## Важные интеграционные риски

### 1. AI
Возможен конфликт между:
- старым TitanAI
- новыми hostile systems
- разными структурами entity state

### 2. Damage model
Возможен конфликт между:
- обычным health damage
- component damage
- vehicle/tank part damage
- status effects

### 3. Save compatibility
После переноса новых систем старые сейвы могут ломаться.

Нужна:
- versioned serialization
- migration path

### 4. Assets
Проблемы:
- разные регистры папок;
- битые относительные пути;
- разные naming conventions;
- отсутствующие текстуры/модели.

### 5. Runtime/editor split
Нужно разделить:
- то, что работает в игре;
- то, что работает как инструмент разработчика.

### 6. Multiplayer
Если переносить Winsock/network:
- не смешивать с single-player logic без слоя абстракции;
- сначала сделать чистую simulation architecture.

---

## Обязательная стратегия реализации

Каждую систему внедрять так:

1. найти исходную реализацию;
2. выделить её зависимости;
3. выделить данные/struct/enum;
4. проверить, есть ли уже аналог в Izometrical_Project;
5. решить:
   - перенос,
   - адаптация,
   - переписывание,
   - отбрасывание явно плохого legacy-кода;
6. внедрить минимальную рабочую версию;
7. протестировать;
8. только потом расширять до полного функционала.

---

## Что нельзя делать

- слепо копировать гигантские legacy-файлы без декомпозиции;
- тащить баги как есть;
- плодить дублирующие gameplay-системы;
- смешивать editor/debug код с production runtime без границ;
- ломать целевой проект ради быстрого переноса;
- считать “частично перенесено” за “готово”.

---

## Что считается завершением работы

Работа считается завершённой, когда:

1. все ключевые механики из обоих исходных репозиториев перенесены;
2. в проекте нет известных критических багов;
3. проект собирается и запускается;
4. геймплей целостный;
5. нет сломанных путей к ресурсам;
6. системы не конфликтуют между собой;
7. есть внятная архитектура;
8. перенос не является набором случайных кусков legacy-кода;
9. итоговый репозиторий — реально рабочая игра, а не полуинтегрированный прототип.

---

## Следующий обязательный шаг

После этого плана нужно сделать следующий файл:

`REPOSITORY_AUDIT_CHECKLIST.md`

В нём будет:
- полный чеклист обследования всех репозиториев,
- что именно смотреть по веткам/коммитам,
- какие подсистемы искать,
- как фиксировать найденное,
- как не потерять механики.

---

## Режим работы дальше

Порядок выдачи файлов:

1. `MERGE_MASTER_PLAN.md`
2. `REPOSITORY_AUDIT_CHECKLIST.md`
3. `MECHANICS_TRANSFER_MATRIX.md`
4. `TARGET_ARCHITECTURE.md`
5. `INTEGRATION_PHASES.md`
6. далее уже кодовые файлы и конкретные патчи

---

## Примечание

Если потребуется, можно параллельно использовать помощь других ИИ/инструментов для:
- аудита большого объёма legacy-кода;
- поиска пропущенных механик;
- сверки архитектуры;
- генерации рефакторинг-плана;
- проверки переносов.

Но итоговая интеграция должна оставаться согласованной и контролируемой в одном плане, без хаотичного смешивания несовместимых решений.