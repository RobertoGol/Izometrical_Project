# REPOSITORY AUDIT CHECKLIST

Цель: полностью обследовать 3 репозитория и ничего не потерять при переносе механик в `Izometrical_Project`.

Репозитории:

1. `GMyGameDoNotTouch`
2. `Video_Game_Izom`
3. `Izometrical_Project`

---

## Главный принцип аудита

Нельзя ограничиваться только текущей веткой и только “главными” файлами.

Нужно проверить:

- все ветки;
- все коммиты;
- все markdown-файлы;
- все крупные cpp/hpp;
- все data/config/assets manifests;
- все экспериментальные/старые реализации, если в них есть нужная механика.

---

# 1. Общий порядок обследования каждого репозитория

Для каждого репозитория пройти:

## 1.1. Ветки
Проверить:

- `main`
- `master`
- все feature-ветки
- все backup/old/test/prototype ветки
- все ветки с gameplay/ai/tank/world/editor/network/building в названии

### Что фиксировать
Для каждой ветки записывать:

- название ветки;
- назначение;
- уникальные механики;
- какие файлы отличаются от основной ветки;
- есть ли там более новые или более полные реализации;
- есть ли временный/грязный код, который нельзя переносить напрямую.

---

## 1.2. Коммиты
Просмотреть историю коммитов и обратить внимание на коммиты с признаками:

- `ai`
- `enemy`
- `hostile`
- `tank`
- `vehicle`
- `combat`
- `weapon`
- `damage`
- `world`
- `editor`
- `tool`
- `network`
- `save`
- `profile`
- `session`
- `weather`
- `stress`
- `story`
- `loot`
- `prefab`
- `terminal`
- `build`
- `camp`
- `obj`
- `grid`
- `optimization`

### Что фиксировать
Для каждого важного коммита:

- hash;
- краткое описание;
- какие механики добавлены;
- какие механики исправлены;
- какие файлы тронуты;
- переносить ли это как источник правды;
- есть ли регрессии после него.

---

## 1.3. Markdown и документация
Проверить:

- `README.md`
- `TODO.md`
- `ROADMAP.md`
- `DESIGN.md`
- `NOTES.md`
- `CHANGELOG.md`
- любые `.md`, `.txt`, `.rtf`, design notes

### Что искать
- описания механик;
- объяснения архитектуры;
- TODO/FIXME;
- список известных багов;
- скрытые фичи, которые есть в планах, но не видны сразу в коде;
- управление, хоткеи, режимы отладки;
- dev-команды и debug flags.

---

## 1.4. Большие runtime-файлы
Особенно внимательно смотреть очень большие файлы:

- гигантские `GameRuntime.cpp`
- `main.cpp`
- `World.cpp`
- `Game.cpp`
- `Engine.cpp`
- `AI.cpp`
- любые файлы на тысячи строк

### Почему это важно
Часто механики не разложены по модулям, а спрятаны внутри:
- update loop;
- switch-case по game state;
- if-блоков debug/gameplay logic;
- локальных static helper functions;
- массивов/таблиц;
- adhoc state machines.

---

# 2. Что искать по категориям систем

---

## 2.1. Core / Engine
Искать:

- точку входа;
- главный цикл;
- инициализацию subsystems;
- input/update/render stages;
- timing/timestep;
- scene/world lifecycle;
- entity registration;
- asset bootstrap;
- shutdown/cleanup.

### Проверить
- нет ли дублирующих loop-систем;
- нет ли старого и нового main loop одновременно;
- нет ли зависимостей gameplay-кода от debug init порядка;
- есть ли deterministic update или частично фиксированный timestep.

---

## 2.2. Player Controller
Искать:

- движение;
- коллизии;
- стрельбу;
- переключение оружия;
- лечение;
- перезарядку;
- инвентарь;
- интеракции;
- special actions;
- stamina/stress/survival;
- seat mount/dismount;
- режим пешком / в технике.

### Проверить
- какие состояния игрока есть;
- есть ли state machine;
- где находятся cooldowns/timers;
- где обрабатывается урон;
- где идёт блокировка управления;
- есть ли конфликт между пешим режимом и режимом техники.

---

## 2.3. Weapon / Combat
Искать:

- типы оружия;
- ballistic / hitscan;
- spread;
- recoil;
- reload;
- ammo pools;
- hit reactions;
- damage types;
- explosion damage;
- shock wave;
- thermal/overheat;
- cockpit fire logic;
- tank weapon logic.

### Проверить
- нет ли разных несогласованных damage pipelines;
- где считается friendly/hostile;
- как устроен line-of-sight;
- как обрабатываются криты/part damage;
- как связаны FX и реальный урон.

---

## 2.4. Enemy / Hostile AI
Искать:

- hostile types;
- TitanAI;
- агро/awareness;
- chase logic;
- ranged/melee attack logic;
- state transitions;
- cover/tactical logic;
- robot control logic;
- pathing;
- target selection;
- cooldowns;
- death/disable states.

### Проверить
- есть ли разные AI-системы, которые конфликтуют;
- где данные AI хранятся;
- есть ли hardcoded параметры;
- можно ли вынести конфиги;
- как работает AI в update loop;
- ломается ли AI при посадке игрока в технику.

---

## 2.5. Vehicle / Titan / Tank
Искать:

- mount/dismount;
- seat swap;
- driver/gunner roles;
- movement;
- turning;
- weapon handling;
- armor/part damage;
- thermal load;
- utility modules;
- hangar interaction;
- repair logic;
- cockpit actions;
- fire-from-cockpit;
- disabled states;
- restore/recover states.

### Проверить
- не размазана ли логика танка по нескольким файлам;
- как разделён input для пешего режима и режима техники;
- где определяются слоты модулей;
- как считается урон по частям;
- как происходит уничтожение/вывод из строя.

---

## 2.6. Damage / Health / Survival
Искать:

- HP;
- armor;
- localized damage;
- limb/part damage;
- stress;
- second wind;
- soul line;
- healing;
- rations;
- buffs/debuffs;
- environmental damage;
- acid rain / weather damage.

### Проверить
- единая ли система статусов;
- нет ли трёх разных способов лечиться;
- где баффы обновляются и снимаются;
- что происходит при критическом HP;
- какие системы влияют друг на друга.

---

## 2.7. World / Interaction
Искать:

- терминалы;
- двери;
- контейнеры;
- переключатели;
- lootables;
- triggers;
- scripted zones;
- archive/garage/exterior events;
- object activation;
- contextual prompts.

### Проверить
- как определяется доступность взаимодействия;
- как хранится состояние мира;
- сохраняются ли события;
- нет ли одноразовых событий без флага завершения;
- где живут scripted sequences.

---

## 2.8. Story / Quest / Event scripting
Искать:

- zone events;
- mission flags;
- progression gates;
- story routes;
- BT-72 recovery;
- clearance stages;
- first combat;
- return sequences;
- archive logic;
- cryo locker logic.

### Проверить
- где живут флаги прогресса;
- сериализуются ли они;
- не ломается ли сюжет при загрузке сейва;
- нет ли жёсткой завязки на порядок посещения зон.

---

## 2.9. Inventory / Loot / Items
Искать:

- item definitions;
- loot tables;
- rarity tiers;
- weapon ammo;
- medical items;
- rations;
- tapes;
- modules;
- service kits;
- crafting/build items.

### Проверить
- где описаны предметы;
- как происходит выдача лута;
- есть ли procedural generation;
- как сохраняется inventory;
- нет ли разных item id систем.

---

## 2.10. Weather / Environment
Искать:

- ether fog;
- acid rain;
- visibility modifiers;
- dot damage;
- post-processing hooks;
- environmental hazards.

### Проверить
- влияет ли погода только визуально или и на gameplay;
- есть ли hooks в AI/perception;
- есть ли hooks в survival/damage;
- как сохраняется погода между сценами/сейвами.

---

## 2.11. Breakables / Physics reactions
Искать:

- стекло;
- ящики;
- растения;
- реактивные объекты;
- impulse;
- debris;
- destructible flags.

### Проверить
- это чисто визуал или влияет на gameplay;
- есть ли collision state после разрушения;
- есть ли drop tables;
- есть ли performance risks.

---

## 2.12. UI / HUD / Terminal / Pip-Pad
Искать:

- HUD;
- inventory ui;
- terminal ui;
- pip-pad;
- themes;
- overlays;
- prompts;
- minimap/log/status displays;
- mount/vehicle HUD.

### Проверить
- какие режимы UI уже есть;
- есть ли переключение экранов/состояний;
- нет ли логики gameplay внутри рендера UI;
- где лежат темы/customization;
- как terminal sync встроен в интерфейс.

---

## 2.13. Build / C.A.M.P. / ToolGun / Editor
Искать:

- placement system;
- build preview;
- snap logic;
- wall/turret/crate placement;
- runtime edit;
- undo/redo;
- validation;
- export;
- semantic authoring;
- toolgun commands.

### Проверить
- это gameplay или dev-tool;
- есть ли сетевые зависимости;
- есть ли ограничения коллизий;
- как объекты сериализуются;
- можно ли использовать это в финальной игре.

---

## 2.14. Save / Load / Profile / Session
Искать:

- profile structs;
- session state;
- world state persistence;
- inventory save;
- quest flags;
- vehicle state;
- player upgrades;
- migration/versioning;
- auto save.

### Проверить
- единая ли сериализация;
- как менялся формат между коммитами;
- есть ли version fields;
- ломаются ли сейвы при добавлении новых полей;
- можно ли сделать migration layer.

---

## 2.15. Audio / Radio / Tape
Искать:

- tape files;
- radio intercepts;
- playback logic;
- subtitle/log hooks;
- collectible audio items.

### Проверить
- как это хранится;
- как запускается воспроизведение;
- влияет ли на сюжет/прогресс;
- есть ли зависимость от asset naming.

---

## 2.16. Skills / Progression
Искать:

- passive skills;
- unlock flags;
- archive sync;
- foot kill;
- tank action;
- stress survival;
- perk application logic.

### Проверить
- где применяются эффекты навыков;
- есть ли уровень/опыт или только флаги;
- сериализуются ли навыки;
- нет ли поломанных зависимостей между skill и combat/survival.

---

## 2.17. Networking / Multiplayer
Искать:

- winsock code;
- lobby;
- chat;
- voice;
- replication ideas;
- player slots;
- session management;
- host/client logic.

### Проверить
- насколько код отделим от single-player;
- есть ли зачатки правильной simulation abstraction;
- нет ли жёсткой зависимости gameplay на networking timing;
- переносить ли сразу или оставить на позднюю фазу.

---

## 2.18. Spatial structures / Optimization
Искать:

- spatial grid;
- partitioning;
- broadphase collision;
- culling helpers;
- ai query acceleration;
- nearby object search.

### Проверить
- совместимо ли это с текущими entity structures;
- нет ли сильной привязки к старому world format;
- даёт ли реальную пользу;
- безопасно ли переносить до стабилизации gameplay.

---

## 2.19. Asset pipeline
Искать:

- пути к текстурам;
- пути к моделям;
- регистр директорий;
- asset manifests;
- procedural texture generation;
- obj loading;
- fallback assets.

### Проверить
- Linux/Windows sensitivity к регистру;
- битые пути;
- старые имена;
- неиспользуемые ассеты;
- runtime asset generation.

---

# 3. Что фиксировать в результате аудита

Для каждой найденной механики нужно записывать:

- название механики;
- репозиторий-источник;
- ветка;
- коммит(ы);
- файлы;
- зависимые файлы;
- краткое описание;
- текущее состояние в `Izometrical_Project`:
  - отсутствует
  - частично
  - устаревшая версия
  - сломана
  - почти готова
  - готова
- решение:
  - переносить как есть
  - адаптировать
  - переписать
  - декомпозировать
  - отложить
- приоритет:
  - P0
  - P1
  - P2
  - P3
- риски интеграции;
- тест-критерий готовности.

---

# 4. Формат карточки механики

Использовать такой шаблон:

## [Название механики]

- Источник: `repo / branch / commit`
- Файлы:
  - `...`
  - `...`
- Зависимости:
  - `...`
- Описание:
  - ...
- Статус в Izometrical_Project:
  - отсутствует / частично / сломана / устарела / готова
- Что делать:
  - перенести / адаптировать / переписать / отложить
- Приоритет:
  - P0 / P1 / P2 / P3
- Основные риски:
  - ...
- Критерий готовности:
  - ...

---

# 5. Обязательная сверка между тремя репозиториями

Нужно обязательно сравнивать:

## 5.1. Есть ли механика только в старом репозитории
Иногда механика есть в старой ветке и исчезла в новой.

## 5.2. Есть ли более новая версия механики в другом репозитории
Например:
- прототип в одном репо,
- улучшенная версия в другом.

## 5.3. Есть ли уже частичный перенос в Izometrical_Project
Тогда нужно понять:
- перенос сделан правильно?
- версия там не устарела?
- не потеряны ли состояния/данные/edge-cases?

## 5.4. Есть ли скрытые зависимости
Например:
- AI зависит от weather;
- tank зависит от damage model;
- zone event зависит от terminal sync;
- stress зависит от heal/rations;
- build system зависит от inventory/items.

---

# 6. Красные флаги при аудите

Если находишь что-то из списка ниже — обязательно помечать отдельно.

## Красные флаги
- giant god object class;
- несколько параллельных систем урона;
- несколько параллельных AI;
- gameplay логика внутри render-only кода;
- hardcoded asset paths;
- hardcoded branch-specific data;
- сломанная сериализация;
- enum mismatch между файлами;
- duplicate struct names с разным смыслом;
- разные item ids в разных файлах;
- временные debug-костыли, влияющие на gameplay;
- неочевидные magic numbers;
- выключенные куски через `#if 0`;
- “мертвые” механики, которые можно оживить;
- код, который компилируется только в одном конкретном окружении.

---

# 7. Минимальные результаты после полного аудита

После завершения аудита обязательно должны появиться следующие документы:

1. `MECHANICS_TRANSFER_MATRIX.md`
2. `TARGET_ARCHITECTURE.md`
3. `INTEGRATION_PHASES.md`
4. `BUG_RISK_REGISTER.md`
5. `SAVE_COMPATIBILITY_PLAN.md`
6. `ASSET_PATH_NORMALIZATION.md`

---

# 8. Практический порядок работы после аудита

Рекомендуемый порядок:

1. собрать полный список механик;
2. сгруппировать по системам;
3. определить зависимости;
4. определить приоритеты;
5. выбрать “источник правды” по каждой системе;
6. спроектировать целевую архитектуру;
7. переносить сначала P0;
8. затем P1;
9. потом P2/P3;
10. после каждой фазы делать regression pass.

---

# 9. Что считается плохим аудитом

Аудит плохой, если:

- просмотрели только текущую ветку;
- не проверили коммиты;
- не выписали зависимости;
- не зафиксировали баги;
- не указали источник правды;
- не отделили “можно перенести как есть” от “нужно переписать”;
- не пометили риски интеграции;
- не описали критерии готовности.

---

# 10. Что считается хорошим аудитом

Аудит хороший, если после него можно:

- без паники переносить системы по очереди;
- не терять механики;
- понимать, где брать код;
- понимать, что надо переписывать;
- видеть порядок интеграции;
- заранее знать риски;
- довести проект до рабочего состояния без хаоса.

---

# Следующий файл

После этого файла нужно сделать:

`MECHANICS_TRANSFER_MATRIX.md`

В нём будет уже конкретная матрица:
- какая механика,
- откуда берётся,
- куда встраивается,
- в каком статусе,
- что делать,
- в каком порядке переносить.