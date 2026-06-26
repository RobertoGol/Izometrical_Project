# BUG RISK REGISTER

Цель: заранее зафиксировать основные технические и геймплейные риски при переносе механик из `GMyGameDoNotTouch` и `Video_Game_Izom` в `Izometrical_Project`.

---

## Шкала оценки

### Severity
- `S0` — косметика
- `S1` — неприятно, но не ломает игру
- `S2` — ломает часть системы
- `S3` — критично ломает gameplay/прогресс/стабильность
- `S4` — блокер сборки, запуска или сохранений

### Probability
- `P0` — маловероятно
- `P1` — возможно
- `P2` — вероятно
- `P3` — почти наверняка

### Mitigation Status
- `OPEN`
- `WATCH`
- `MITIGATING`
- `RESOLVED`

---

# 1. Build / Compile Risks

## R-001 — Конфликт версий `TitanAI.hpp`
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Новая версия `TitanAI.hpp` содержит дополнительные возможности:
- `tryMount / dismount`
- котёл / boiler-related logic
- stress hooks
- `fireFromCockpit`
- Core-related logic

Старая версия в целевом проекте короче и архитектурно отличается.

### Риск
После замены заголовка:
- могут не совпасть сигнатуры;
- могут отсутствовать требуемые поля/типы;
- `main.cpp` или другие файлы могут ожидать старый API.

### Митигация
- обновлять вместе с зависящими файлами;
- сразу проверять compile errors;
- зафиксировать API diff до интеграции;
- отдельно проверить includes и forward declarations.

---

## R-002 — Конфликт новой и старой версии `src/main.cpp`
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Упоминалась новая версия `main.cpp`, заметно отличающаяся по структуре.

### Риск
- ожидание новых модулей/типов;
- удаление старых зависимостей, которые всё ещё нужны;
- рассинхрон с текущим runtime/bootstrap.

### Митигация
- сравнить старый и новый `main.cpp` посекционно;
- отдельно выписать:
  - removed logic
  - moved logic
  - new dependencies
- не просто overwrite, а делать controlled merge.

---

## R-003 — Сломанные include paths после декомпозиции
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
При переносе giant-file логики в новые модули почти наверняка сломаются include-зависимости.

### Риск
- циклические include;
- missing types;
- duplicate declarations;
- ошибки из-за порядка include.

### Митигация
- использовать forward declarations;
- разделять interface / implementation;
- не тащить всё через один mega-header;
- после каждого шага делать compile pass.

---

# 2. Asset / Resource Risks

## R-004 — Проблема регистра папок `Enemies` / `Vehicles`
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Есть конфликт:
- `assets/Enemies/`
- `assets/Vehicles/`

vs ожидаемые:
- `assets/enemies/`
- `assets/vehicles/`

### Риск
На чувствительных к регистру системах ресурсы просто не будут грузиться.

### Митигация
- принять единый стандарт именования;
- переименовать каталоги;
- обновить все пути в коде и data;
- сделать документ `ASSET_PATH_NORMALIZATION.md`;
- добавить runtime logging missing assets.

---

## R-005 — Битые относительные пути после переноса ассетов
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
После переноса ресурсов старые относительные пути могут стать невалидными.

### Риск
- отсутствующие текстуры;
- отсутствующие модели;
- пустые спрайты;
- падения/ошибки при загрузке.

### Митигация
- провести path-audit;
- ввести централизованный asset resolver;
- использовать fallback assets;
- логировать missing path с контекстом.

---

## R-006 — Asset naming mismatch между репозиториями
- Severity: `S2`
- Probability: `P2`
- Status: `OPEN`

### Описание
Один и тот же ресурс или тип ресурса может называться по-разному в разных репозиториях.

### Митигация
- составить таблицу old → new path/name;
- не хранить магические строки в gameplay-коде;
- по возможности вынести пути в config/content layer.

---

# 3. Runtime / Architecture Risks

## R-007 — Giant main loop содержит gameplay, который потеряется при рефакторинге
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Часть логики может быть спрятана прямо в `main.cpp` или больших runtime-файлах.

### Риск
После “очистки” main loop исчезнет:
- часть input logic;
- часть state transitions;
- часть AI/world updates;
- часть debug/gameplay bridges.

### Митигация
- не переписывать вслепую;
- делать mapping: old loop section → new owner system;
- переносить логику только после фиксации ответственности.

---

## R-008 — Несколько конфликтующих gameplay pipelines одновременно
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Есть риск, что в проекте одновременно живут:
- старый player pipeline;
- новый player pipeline;
- старый vehicle pipeline;
- новый damage path.

### Риск
- одни и те же события обрабатываются дважды;
- урон применяется дважды или не применяется;
- состояния игрока расходятся.

### Митигация
- вводить owner system для каждой области;
- удалять/гасить legacy path после миграции;
- не держать 2 активные реализации долго.

---

## R-009 — Giant runtime blocks нельзя безопасно копировать кусками
- Severity: `S3`
- Probability: `P3`
- Status: `OPEN`

### Описание
Большие legacy-файлы часто содержат скрытые локальные зависимости.

### Риск
Вырезанный кусок:
- теряет helper-функции;
- теряет локальные state assumptions;
- ломает update order.

### Митигация
- сначала декомпозиция;
- потом перенос;
- обязательно фиксировать скрытые зависимости.

---

# 4. State / Save / Persistence Risks

## R-010 — Отсутствие versioned save schema
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
После добавления новых систем старые сохранения без version field почти наверняка сломаются.

### Риск
- не загружается сейв;
- загружается с мусором;
- поля смещаются;
- мир/сюжет становятся неконсистентными.

### Митигация
- ввести format version;
- ввести migration strategy;
- safe defaults для новых полей;
- документ `SAVE_COMPATIBILITY_PLAN.md`.

---

## R-011 — Story flags не сериализуются полностью
- Severity: `S4`
- Probability: `P2`
- Status: `OPEN`

### Описание
Zone events и сюжетные переходы могут зависеть от множества флагов.

### Риск
После загрузки:
- события повторяются;
- события не запускаются;
- прогресс блокируется;
- игра застревает в неправильном state.

### Митигация
- централизовать story flags;
- явный список сериализуемых флагов;
- тест-кейсы save/load around story events.

---

## R-012 — World persistence для breakables/build/loot неполная
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
После добавления разрушаемых объектов, строительства и лута мир станет stateful.

### Риск
- ящик “возрождается” после загрузки;
- поставленная турель исчезает;
- разрушенное стекло снова целое;
- собранный лут появляется снова без правил.

### Митигация
- persistence key для world objects;
- отдельные state tables для break/build/loot;
- сохранять только необходимое, но стабильно.

---

# 5. Combat / Damage Risks

## R-013 — Несколько разных систем урона
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Обычный урон, урон по технике, part damage, stress, взрывной урон, погодный урон могут жить в разных местах.

### Риск
- несогласованный результат;
- двойной урон;
- отсутствие реакций;
- разные правила смерти/disable.

### Митигация
- единый DamageSystem;
- единый request/apply path;
- разграничить hit detection и final damage resolution.

---

## R-014 — Mechanical hostile damage конфликтует с baseline HP model
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Роботы/механические враги повреждаются не просто по HP, а по подсистемам.

### Риск
- враг жив/мертв в разных системах по-разному;
- AI теряет оружие, но всё ещё стреляет;
- mobility сломана, но враг движется как обычно.

### Митигация
- entity damage state должен поддерживать component states;
- AI должен читать эти component states.

---

## R-015 — Shock wave ломает aim/control states
- Severity: `S2`
- Probability: `P2`
- Status: `OPEN`

### Описание
Ударная волна может вмешиваться в прицеливание, анимацию и управление.

### Митигация
- выделить shock/stagger как временный статус;
- не модифицировать input хаотично напрямую;
- ограничить duration и stacking rules.

---

## R-016 — Heal / Reload действия конфликтуют с movement/vehicle states
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Лечение и перезарядка — это stateful actions.

### Риск
- можно лечиться во время недопустимого действия;
- reload обрывается неочевидно;
- vehicle mode не блокирует несовместимые действия.

### Митигация
- явная state machine player actions;
- allow/deny matrix по состояниям;
- тест-кейсы:
  - reload while moving
  - heal while mounting
  - reload in vehicle
  - interrupted action behavior

---

# 6. AI Risks

## R-017 — Awareness/Aggro логика теряется при переносе
- Severity: `S3`
- Probability: `P3`
- Status: `OPEN`

### Описание
Если переносить только “враг бежит и бьёт”, можно потерять стадию обнаружения.

### Риск
AI становится тупым или слишком агрессивным.

### Митигация
- отдельно выделить perception;
- отдельно awareness meter/state;
- отдельно engage/disengage rules.

---

## R-018 — AI ломается при взаимодействии с техникой игрока
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Игрок может быть:
- пешком;
- водителем;
- стрелком;
- в процессе mount/dismount.

### Риск
AI:
- не видит игрока в технике;
- видит сразу два таргета;
- атакует не ту сущность;
- застревает в state transition.

### Митигация
- единая target abstraction;
- player presence rules for mounted state;
- отдельные тесты on-foot vs in-vehicle.

---

## R-019 — Tactical AI завязан на старый world query API
- Severity: `S2`
- Probability: `P2`
- Status: `OPEN`

### Описание
HumanTactical и подобные AI могут использовать старые world helpers.

### Митигация
- выделить нужные query APIs;
- адаптировать их в world layer;
- не тащить старый world coupling как есть.

---

# 7. Vehicle Risks

## R-020 — Seat swap ломает owner/input state
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Пересадка между местами меняет доступные действия и источник управления.

### Риск
- управление остаётся на старом месте;
- HUD не обновляется;
- действия стрелка доступны водителю и наоборот.

### Митигация
- VehicleSeatSystem как отдельный owner;
- role capabilities matrix;
- atomic seat transition.

---

## R-021 — Vehicle subsystem damage не синхронизирован с управлением
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Если повреждена мобильность/оружие/сенсоры, поведение техники должно измениться.

### Риск
- техника едет без гусениц;
- стреляет сломанным оружием;
- HUD не отражает поломку.

### Митигация
- единый vehicle state;
- vehicle control reads subsystem health;
- cockpit/UI hooks из того же источника данных.

---

## R-022 — Utility modules требуют item/module schema, которой ещё нет
- Severity: `S2`
- Probability: `P2`
- Status: `OPEN`

### Описание
Bucket Rig / Ram Shield / Tow Coupler не должны быть просто хардкодными флагами.

### Митигация
- сперва определить module slot model;
- затем equipped/activation state;
- затем item/hangar integration.

---

# 8. Story / Event Risks

## R-023 — Scripted zone events завязаны на порядок выполнения
- Severity: `S4`
- Probability: `P2`
- Status: `OPEN`

### Описание
Старые сюжетные блоки могут предполагать жёсткую последовательность посещения мест.

### Риск
- soft lock;
- событие не активируется;
- событие активируется раньше времени;
- двойной запуск.

### Митигация
- явные preconditions/postconditions;
- единый StoryEventSystem;
- тестовая таблица всех переходов.

---

## R-024 — Terminal sync и story sync разнесены по разным местам
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Терминалы могут одновременно быть:
- UI событием;
- world interaction;
- story progression hook.

### Митигация
- терминал как interaction source;
- story effect через event dispatch;
- не связывать сюжет напрямую с отрисовкой терминала.

---

# 9. Inventory / Item Risks

## R-025 — Несколько item id схем
- Severity: `S4`
- Probability: `P2`
- Status: `OPEN`

### Описание
В разных репозиториях предметы могут кодироваться по-разному.

### Риск
- не тот предмет используется;
- не грузится инвентарь;
- баффы привязываются к неправильным айтемам.

### Митигация
- единая item definition table;
- migration mapping old ids → new ids;
- не хранить поведение по сырым магическим id.

---

## R-026 — Loot generator не согласован с inventory stack rules
- Severity: `S2`
- Probability: `P2`
- Status: `OPEN`

### Описание
Лут может генерировать предметы, которые не вписываются в новую item schema.

### Митигация
- сначала item schema;
- потом loot generation adaptation;
- validation на spawn item.

---

# 10. Build / Editor / Tools Risks

## R-027 — ToolGun/editor код вмешивается в production gameplay
- Severity: `S2`
- Probability: `P2`
- Status: `OPEN`

### Описание
Runtime tools иногда живут прямо в обычном игровом цикле.

### Риск
- случайный доступ игрока к dev-функциям;
- странные состояния мира;
- лишние зависимости.

### Митигация
- compile/runtime flags;
- выделение tools layer;
- ограничение через dev mode.

---

## R-028 — Build placements не сохраняются корректно
- Severity: `S3`
- Probability: `P2`
- Status: `OPEN`

### Описание
Строительство требует world persistence.

### Митигация
- stable build object ids;
- placement serialization schema;
- save/load tests for builds.

---

# 11. Networking Risks

## R-029 — Слишком ранняя интеграция сети ломает архитектуру
- Severity: `S3`
- Probability: `P3`
- Status: `OPEN`

### Описание
Подключение networking до стабилизации single-player обычно размножает сложность в разы.

### Митигация
- network только после stable runtime;
- отдельный слой абстракции;
- сначала чистый single-player model.

---

# 12. Testing Risks

## R-030 — Нет regression discipline после фаз интеграции
- Severity: `S4`
- Probability: `P3`
- Status: `OPEN`

### Описание
Даже хорошие переносы быстро ломаются без повторных проверок.

### Митигация
После каждой фазы проверять минимум:
- compile
- launch
- asset loading
- movement
- combat baseline
- mount/dismount
- save/load
- story flag sanity

---

# 13. Критические риски первого приоритета

Ниже список того, что надо закрывать первым.

## Priority-1 Risk Set
1. `R-001` — конфликт `TitanAI.hpp`
2. `R-002` — конфликт `main.cpp`
3. `R-004` — asset case sensitivity
4. `R-007` — потеря логики в main/runtime loop
5. `R-008` — дублирующие gameplay pipelines
6. `R-010` — отсутствие versioned save schema
7. `R-013` — несколько систем урона
8. `R-023` — сюжетные зоны могут soft-lock’ать игру
9. `R-025` — несколько item id схем
10. `R-030` — отсутствие regression discipline

---

# 14. Минимальный шаблон обновления риска

Использовать такой формат при сопровождении проекта:

## [Risk ID] Название
- Severity:
- Probability:
- Status:
- Owner:
- Trigger:
- Impact:
- Current Mitigation:
- Next Action:
- Validation Needed:

---

# 15. Когда считать риск закрытым

Риск считается `RESOLVED`, только если:

1. есть конкретное техническое решение;
2. решение внедрено;
3. compile/run подтверждены;
4. нет воспроизведения бага в ключевых сценариях;
5. не появилось новой критичной регрессии.

---

# Следующий файл

После этого нужен:

`SAVE_COMPATIBILITY_PLAN.md`

В нём нужно описать:
- схему сохранений;
- versioning;
- migration;
- как сохранять story/world/vehicle/inventory/build/breakables;
- как не ломать старые сейвы.